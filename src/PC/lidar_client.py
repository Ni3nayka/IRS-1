import socket, json, threading, queue, math, time
import numpy as np


# ---------- geometry + RANSAC helpers ----------

def polar_to_cartesian(angles, distances):
    x = distances * np.cos(angles)
    y = distances * np.sin(angles)
    return x, y

def normalize_line(a, b, c):
    n = math.hypot(a, b)
    if n == 0: return None
    a /= n; b /= n; c /= n
    if -c < 0: a, b, c = -a, -b, -c
    return a, b, c

def line_pca_fit(points):
    if len(points) < 2: return None
    pts = np.asarray(points)
    ctr = pts.mean(axis=0)
    Q = pts - ctr
    _, _, vh = np.linalg.svd(Q, full_matrices=False)
    dirv = vh[0]
    nx, ny = -dirv[1], dirv[0]
    a, b, c = nx, ny, -(nx*ctr[0] + ny*ctr[1])
    return normalize_line(a, b, c)

def detect_walls_ransac(x, y, max_lines=4, dist_thresh=0.05, min_points=20, max_iters=400):
    pts = np.column_stack((x, y))
    valid = np.isfinite(pts).all(axis=1)
    pts = pts[valid]
    if len(pts) < min_points: return []

    remaining = np.arange(len(pts))
    walls = []
    rng = np.random.default_rng()

    for _ in range(max_lines):
        if len(remaining) < min_points:
            break
        best_inliers = None
        best_model = None

        for _ in range(max_iters):
            if len(remaining) < 2: break
            i = rng.choice(remaining, size=2, replace=False)
            p1, p2 = pts[i[0]], pts[i[1]]
            v = p2 - p1
            if np.hypot(v[0], v[1]) < 1e-6: continue
            a, b = -v[1], v[0]
            c = -(a*p1[0] + b*p1[1])
            model = normalize_line(a, b, c)
            if model is None: continue
            a, b, c = model
            d = np.abs(a*pts[remaining,0] + b*pts[remaining,1] + c)
            m = d < dist_thresh
            n_in = int(np.sum(m))
            if n_in >= min_points and (best_inliers is None or n_in > len(best_inliers)):
                best_inliers = remaining[m]
                best_model = (a, b, c)

        if best_inliers is None:
            break

        refined = line_pca_fit(pts[best_inliers])
        if refined is None:
            break
        a, b, c = refined
        walls.append({'abc': (a,b,c), 'points': pts[best_inliers]})

        keep = np.ones(len(remaining), bool)
        rem_mask = np.isin(remaining, best_inliers)
        keep[rem_mask] = False
        remaining = remaining[keep]

    return walls

def line_to_polar(a,b,c):
    rho = -c
    phi = math.atan2(b,a)
    return rho, phi

def wall_orientation_deg(a,b):
    # acute angle between wall and lidar front side (x-axis)
        return math.degrees(math.atan2(abs(a), abs(b)))

def wrap_pi(a): return (a + math.pi) % (2*math.pi) - math.pi

def classify_wall(phi, front_deg=30.0, side_deg=45.0):
    ang = wrap_pi(phi)
    if abs(ang) <= math.radians(front_deg):
        return 0  # FRONT
    if -math.pi <= ang < -math.radians(front_deg):
        if ang < -math.radians(180 - side_deg):
            return 2  # BACK
        return 3      # LEFT  (swapped)
    if ang > math.radians(front_deg):
        if ang > math.radians(180 - side_deg):
            return 2  # BACK
        return 1      # RIGHT (swapped)
    return 2          # BACK

# ---------- client + background thread ----------

class _LidarClient:
    def __init__(self, host, port, distance_scale=0.001):
        self.host = host; self.port = port
        self.distance_scale = distance_scale
        self.sock = None
        self.queue = queue.Queue(maxsize=4)
        self.running = False
        self.thr = None

    def connect(self):
        self.sock = socket.create_connection((self.host, self.port))
        self.running = True
        self.thr = threading.Thread(target=self._reader, daemon=True)
        self.thr.start()

    def close(self):
        self.running = False
        try:
            if self.sock: self.sock.shutdown(socket.SHUT_RDWR)
        except Exception: pass
        try:
            if self.sock: self.sock.close()
        except Exception: pass
        self.sock = None

    def _reader(self):
        f = self.sock.makefile('r', encoding='utf-8', newline='\n')
        while self.running:
            line = f.readline()
            if not line: break
            try:
                pkt = json.loads(line)
                pts = pkt.get('points', [])
                out = [(math.radians(a), float(d)*self.distance_scale)
                       for a,d in pts]
                if not self.queue.empty():
                    try: self.queue.get_nowait()
                    except queue.Empty: pass
                self.queue.put_nowait(out)
            except Exception:
                continue


# ---------- main lidar wrapper ----------

class Lidar:
    def __init__(self, host='127.0.0.1', port=9999,
                 distance_scale=0.001, fps=10):
        self.wall_dist   = [math.nan]*4   # [front, right, back, left]
        self.wall_angles = [math.nan]*4   # [front, right, back, left]
        self._client = _LidarClient(host, port, distance_scale)
        self._fps = fps
        self._stop = False
        self._thr = threading.Thread(target=self._wall_navigation, daemon=True)
        self._thr.start()

    def stop(self):
        self._stop = True
        self._client.close()

    def _wall_navigation(self):
        try:
            self._client.connect()
        except Exception as e:
            print("Failed to connect:", e)
            return

        while not self._stop:
            try:
                pts = self._client.queue.get(timeout=1.0)
            except queue.Empty:
                continue
            if not pts:
                continue

            th, r = zip(*pts)
            th = np.asarray(th, float)
            r = np.asarray(r, float)
            x, y = polar_to_cartesian(th, r)

            # detect
            walls = detect_walls_ransac(x, y,
                                        max_lines=4,
                                        dist_thresh=0.04,
                                        min_points=18,
                                        max_iters=600)

            # reset
            dist_tmp = [math.nan]*4
            angle_tmp = [math.nan]*4

            for w in walls:
                a,b,c = w['abc']
                rho, phi = line_to_polar(a,b,c)
                idx = classify_wall(phi)
                dist_tmp[idx] = rho
                angle_tmp[idx] = wall_orientation_deg(a,b)

            self.wall_dist = dist_tmp
            self.wall_angles = angle_tmp

            time.sleep(1.0 / self._fps)

        self._client.close()


if __name__ == "__main__":
    lidar = Lidar('192.168.43.56', 9999)

    try:
        while True:
            print("Front/Right/Back/Left distances:", lidar.wall_dist)
            print("Front/Right/Back/Left angles:   ", lidar.wall_angles)
            time.sleep(0.5)
    except KeyboardInterrupt:
        lidar.stop()