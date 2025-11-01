import socket
import json
import math
import time
import threading
import numpy as np

# -------------------- geometry helpers --------------------

def polar_to_cartesian(angles, distances):
    x = distances * np.cos(angles)
    y = distances * np.sin(angles)
    return x, y

def normalize_line(a, b, c):
    n = math.hypot(a, b)
    if n == 0:
        return None
    a /= n; b /= n; c /= n
    # make ρ >= 0 (i.e., -c >= 0) by flipping normal if needed
    if -c < 0:
        a = -a; b = -b; c = -c
    return a, b, c

def line_pca_fit(points):
    # total least squares: fit ax+by+c=0 with ||[a,b]||=1
    if len(points) < 2:
        return None
    pts = np.asarray(points)
    centroid = pts.mean(axis=0)
    Q = pts - centroid
    # smallest singular vector of Q gives normal
    _, _, vh = np.linalg.svd(Q, full_matrices=False)
    # direction = first PC, normal = perpendicular
    direction = vh[0]
    nx, ny = -direction[1], direction[0]
    a, b, c = nx, ny, -(nx*centroid[0] + ny*centroid[1])
    return normalize_line(a, b, c)

def line_intersection(l1, l2):
    a1,b1,c1 = l1
    a2,b2,c2 = l2
    det = a1*b2 - a2*b1
    if abs(det) < 1e-9:
        return None
    x = (b1*(-c2) - b2*(-c1)) / det
    y = (a2*(-c1) - a1*(-c2)) / det
    return np.array([x, y], dtype=float)

def line_to_polar(a, b, c):
    # normalized line: ax+by+c=0, ||[a,b]||=1
    rho = -c
    phi = math.atan2(b, a) % (2*math.pi)
    return rho, phi

def ang_diff(a, b):
    d = (a - b + math.pi) % (2*math.pi) - math.pi
    return abs(d)

# -------------------- wall detection (RANSAC, vertical-safe) --------------------

def detect_walls_ransac(x, y, max_lines=4, dist_thresh=30.0, min_points=20, max_iters=400):
    pts = np.column_stack((x, y))
    remaining = np.arange(len(pts))
    walls = []

    for _ in range(max_lines):
        if len(remaining) < min_points:
            break

        best_inliers = None
        best_model = None

        for _ in range(max_iters):
            # sample two distinct points
            idx = np.random.choice(remaining, size=2, replace=False)
            p1, p2 = pts[idx[0]], pts[idx[1]]
            # compute line through p1,p2 in ax+by+c=0
            vx, vy = p2 - p1
            if math.hypot(vx, vy) < 1e-6:
                continue
            # normal to direction (vx,vy)
            a, b = -vy, vx
            c = -(a*p1[0] + b*p1[1])
            model = normalize_line(a, b, c)
            if model is None:
                continue
            a, b, c = model

            # distances to this line
            d = np.abs(a*pts[remaining,0] + b*pts[remaining,1] + c)  # ||n||=1
            inlier_mask = d < dist_thresh
            n_inliers = int(np.sum(inlier_mask))
            if n_inliers >= min_points and (best_inliers is None or n_inliers > len(best_inliers)):
                best_inliers = remaining[inlier_mask]
                best_model = (a, b, c)

        if best_inliers is None:
            break

        # refine with PCA TLS on inliers
        refined = line_pca_fit(pts[best_inliers])
        if refined is None:
            break
        a,b,c = refined

        # store wall
        inlier_points = pts[best_inliers]
        walls.append({
            'abc': (a, b, c),
            'points': inlier_points
        })

        # remove inliers for next lines
        mask_remain = np.ones(len(remaining), dtype=bool)
        mask_remain[np.isin(remaining, best_inliers)] = False
        remaining = remaining[mask_remain]

    return walls

def merge_walls_polar(walls, rho_eps=120.0, phi_eps_deg=5.0):
    if not walls:
        return []
    rhos = []
    phis = []
    for w in walls:
        a,b,c = w['abc']
        rho, phi = line_to_polar(a,b,c)
        rhos.append(rho)
        phis.append(phi)
    rhos = np.array(rhos, dtype=float)
    phis = np.array(phis, dtype=float)

    phi_eps = math.radians(phi_eps_deg)
    used = np.zeros(len(walls), dtype=bool)
    merged = []

    for i in range(len(walls)):
        if used[i]:
            continue
        group = [i]
        for j in range(i+1, len(walls)):
            if used[j]:
                continue
            if abs(rhos[i] - rhos[j]) <= rho_eps and ang_diff(phis[i], phis[j]) <= phi_eps:
                group.append(j)
        for k in group:
            used[k] = True

        # average (ρ, φ) safely
        rho_avg = float(np.mean(rhos[group]))
        cx = np.mean(np.cos(phis[group]))
        sy = np.mean(np.sin(phis[group]))
        phi_avg = math.atan2(sy, cx) % (2*math.pi)

        # rebuild a,b,c from (ρ, φ): n = (cosφ, sinφ), c = -ρ
        a, b, c = math.cos(phi_avg), math.sin(phi_avg), -rho_avg
        merged.append({'abc': (a, b, c), 'points': np.empty((0,2))})

    return merged

# -------------------- yaw + pose --------------------

class YawIntegrator:
    def __init__(self, initial_yaw=0.0, alpha=0.3, max_yaw_change=30.0):
        self.current_yaw = initial_yaw
        self.alpha = alpha
        self.max_yaw_change = max_yaw_change
        self.initialized = False

    def update_from_walls(self, walls, confidence=1.0):
        # walls: list of {'abc':(a,b,c), ...}
        if len(walls) < 2:
            return self.current_yaw

        new_yaw = self._calculate_yaw_from_walls(walls)
        if not self.initialized:
            self.current_yaw = new_yaw
            self.initialized = True
            return self.current_yaw

        eff_alpha = self.alpha * max(0.0, min(1.0, confidence))
        cur = math.radians(self.current_yaw)
        new = math.radians(new_yaw)
        diff = new - cur
        while diff > math.pi: diff -= 2*math.pi
        while diff < -math.pi: diff += 2*math.pi
        diff = np.clip(diff, -math.radians(self.max_yaw_change), math.radians(self.max_yaw_change))
        self.current_yaw = (math.degrees(cur + eff_alpha * diff)) % 360.0
        return self.current_yaw

    def _calculate_yaw_from_walls(self, walls):
        # use wall normals φ to align with 0/90 grid
        wall_angles = []
        for w in walls:
            a,b,_ = w['abc']
            wall_angles.append(math.atan2(b, a))  # normal angle

        best_yaw = 0.0
        best_err = float('inf')
        for test_deg in range(0, 180):  # 1° grid
            t = math.radians(test_deg)
            err = 0.0
            for ang in wall_angles:
                aligned = (ang - t) % (math.pi/2)
                if aligned > math.pi/4:
                    aligned = math.pi/2 - aligned
                err += aligned
            if err < best_err:
                best_err = err
                best_yaw = float(test_deg)
        return best_yaw

    def get_yaw(self):
        return self.current_yaw

    def reset(self, yaw=0.0):
        self.current_yaw = yaw
        self.initialized = True

class PoseEstimator:
    def __init__(self, initial_yaw=0.0, initial_pos=(0, 0)):
        self.yaw_integrator = YawIntegrator(initial_yaw)
        self.position = np.array(initial_pos, dtype=float)
        self.initialized = False

    def update_pose(self, walls, position_confidence=1.0):
        if len(walls) < 2:
            return self._get_current_pose(0.0)

        confidence = self._calculate_confidence(walls)
        yaw = self.yaw_integrator.update_from_walls(walls, confidence)

        pos = self._estimate_position(walls, yaw, position_confidence)
        if pos is not None:
            self.position = pos
        return self._get_current_pose(confidence)

    def _estimate_position(self, walls, yaw, confidence):
        # take first perpendicular pair
        angles = [math.atan2(w['abc'][1], w['abc'][0]) for w in walls]  # normals
        pairs = []
        for i in range(len(walls)):
            for j in range(i+1, len(walls)):
                if abs(ang_diff(angles[i], angles[j]) - math.pi/2) < math.radians(15):
                    pairs.append((i, j))
        if not pairs:
            return self.position

        i, j = pairs[0]
        corner = line_intersection(walls[i]['abc'], walls[j]['abc'])
        if corner is None:
            return self.position

        # naive offset 0.5m from the corner (tune to your environment)
        est = corner + np.array([500.0, 500.0])
        if not self.initialized:
            self.initialized = True
            return est
        w = max(0.0, min(1.0, confidence))
        return self.position + w * (est - self.position)

    def _calculate_confidence(self, walls):
        if len(walls) < 2:
            return 0.0
        base = min(1.0, len(walls)/4.0)
        angles = [math.atan2(w['abc'][1], w['abc'][0]) for w in walls]
        score = 0.0
        for i in range(len(angles)):
            for j in range(i+1, len(angles)):
                if abs(ang_diff(angles[i], angles[j]) - math.pi/2) < math.radians(20):
                    score += 0.2
        return min(1.0, base + score)

    def _get_current_pose(self, confidence):
        return {
            'x': float(self.position[0]),
            'y': float(self.position[1]),
            'yaw': float(self.yaw_integrator.get_yaw()),
            'confidence': float(confidence)
        }

    def get_pose(self):
        return self._get_current_pose(0.0)

# -------------------- client loop --------------------

def _write_fixed4(src_list, target_list, fill=math.nan):
    out = (list(src_list) + [fill]*4)[:4]  # pad or truncate to 4
    target_list[:] = out                    # in-place overwrite (thread-safe enough)

def wall_navigation(yaw_ptr, pose_ptr, wall_distances_ptr, wall_angles_ptr,
                    host='localhost', port=9999,
                    max_lines=4, dist_thresh=30.0, min_points=20, merge=True):
    pose_estimator = PoseEstimator(initial_yaw=0.0, initial_pos=(0.0, 0.0))
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    buffer = ""

    try:
        client_socket.connect((host, port))
        print(f"Connected to LiDAR server at {host}:{port}")
        print("Waiting for LiDAR data...")

        while True:
            data = client_socket.recv(4096).decode('utf-8')
            if not data:
                break
            buffer += data
            lines = buffer.split('\n')
            buffer = lines[-1]

            for line in lines[:-1]:
                s = line.strip()
                if not s:
                    continue
                try:
                    scan = json.loads(s)
                    points = scan.get('points', [])
                    if len(points) < min_points:
                        wall_distances_ptr[0] = []
                        wall_angles_ptr[0] = []
                        continue

                    angles = np.radians([a for (a, d) in points])
                    distances = np.array([d for (a, d) in points], dtype=float)

                    mask = (distances > 0.0) & (distances < 12000.0) & np.isfinite(distances)
                    if np.count_nonzero(mask) < min_points:
                        wall_distances_ptr[0] = []
                        wall_angles_ptr[0] = []
                        continue

                    x, y = polar_to_cartesian(angles[mask], distances[mask])

                    walls = detect_walls_ransac(
                        x, y,
                        max_lines=max_lines,
                        dist_thresh=dist_thresh,
                        min_points=min_points
                    )

                    # optional dedup
                    if merge and len(walls) > 1:
                        walls = merge_walls_polar(walls, rho_eps=120.0, phi_eps_deg=5.0)

                    # update pose (requires lines)
                    pose = pose_estimator.update_pose(walls, position_confidence=0.8)
                    pose_ptr[0] = pose['x']
                    pose_ptr[1] = pose['y']       # FIX: was pose['x']
                    yaw_ptr[0]  = pose['yaw']

                    # export one (ρ, φ) per wall, angles in degrees
                    rhos, phis = [], []
                    for w in walls:
                        a,b,c = w['abc']
                        rho, phi = line_to_polar(a,b,c)
                        rhos.append(float(rho))
                        phis.append(math.degrees(phi))
                    while len(rhos) < 4:
                        rhos.append(None)
                    while len(phis) < 4:
                        phis.append(0.0)

                    _write_fixed4(rhos, wall_distances_ptr)
                    _write_fixed4([math.degrees(p) for p in phis], wall_angles_ptr)

                except json.JSONDecodeError:
                    print("Invalid JSON from server (skipped)")
                except Exception as e:
                    print(f"Processing error: {e}")

            time.sleep(0.01)

    except KeyboardInterrupt:
        print("\nDisconnecting (KeyboardInterrupt)...")
        print("Final pose:", pose_estimator.get_pose())
    except Exception as e:
        print(f"Client error: {e}")
    finally:
        try:
            client_socket.close()
        except:
            pass

# -------------------- convenience wrapper --------------------

class Lidar:
    def __init__(self, host='localhost', port=9999):
        self.yaw = [0.0]              # keep as 1-slot list (mutable)
        self.pose = [0.0, 0.0]        # keep as 2-slot list (mutable)
        self.wall_dist = [math.nan]*4   # <<< flat list, 4 elements
        self.wall_angles = [math.nan]*4 # <<< flat list, 4 elements
        self.thr = threading.Thread(
            target=wall_navigation,
            args=(self.yaw, self.pose, self.wall_dist, self.wall_angles, host, port),
            daemon=True
        )
        self.thr.start()

    def print(self):
        print("угол абсолютный:", self.yaw[0])
        print("позиция робота: ", self.pose)
        print("расстояния до стен:", self.wall_dist)
        print("углы до стен:      ", self.wall_angles)

# -------------------- run standalone --------------------

if __name__ == "__main__":
    yaw = [0.0]
    pose = [0.0, 0.0]
    wall_dist = [math.nan] * 4
    wall_angles = [math.nan] * 4

    thr = threading.Thread(target=wall_navigation, args=(yaw, pose, wall_dist, wall_angles), daemon=True)
    thr.start()

    try:
        while True:
            print(wall_angles)
            # print(
            #     f"yaw={yaw[0]:6.1f}°  pose=({pose[0]:7.1f},{pose[1]:7.1f})  "
            #     f"walls: {len(wall_dist[0])} -> ρ={np.round(wall_dist[0],1)}  φ={np.round(wall_angles[0],1)}°"
            # )
            time.sleep(1.0)
    except KeyboardInterrupt:
        pass
