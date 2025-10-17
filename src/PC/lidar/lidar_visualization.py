import signal, threading
import numpy as np
import matplotlib.pyplot as plt
from rplidar import RPLidar

PORT = '/dev/ttyUSB0'
MIN_QUALITY = 0
MIN_DIST_MM = 50
MAX_DIST_MM = 6000

class LidarReader(threading.Thread):
    def __init__(self, port):
        super().__init__(daemon=True)
        self.lidar = RPLidar(port)
        self._lock = threading.Lock()
        self._latest = (np.empty(0), np.empty(0))  # thetas, dists
        self.running = True
        self.error = None

    def run(self):
        try:
            # iter_scans yields a full revolution: [(q, angle, dist), ...]
            for scan in self.lidar.iter_scans():
                if not self.running: break
                if not scan: continue

                # filter + convert to numpy in one pass
                qs, angs, dists = zip(*scan)
                qs = np.asarray(qs)
                angs = np.asarray(angs, dtype=float)
                dists = np.asarray(dists, dtype=float)

                mask = (qs >= MIN_QUALITY) & (dists > 0)
                if MIN_DIST_MM: mask &= (dists >= MIN_DIST_MM)
                if MAX_DIST_MM: mask &= (dists <= MAX_DIST_MM)

                if not np.any(mask):
                    continue

                thetas = np.deg2rad(angs[mask])
                dists = dists[mask]

                # atomic swap: only the newest revolution is kept
                with self._lock:
                    self._latest = (thetas, dists)
        except Exception as e:
            self.error = e
        finally:
            for fn in (self.lidar.stop, self.lidar.stop_motor, self.lidar.disconnect):
                try: fn()
                except Exception: pass

    def get_latest(self):
        with self._lock:
            return self._latest

    def stop(self): self.running = False

def main():
    reader = LidarReader(PORT)
    reader.start()

    plt.ion()
    fig = plt.figure(figsize=(7, 7))
    ax = fig.add_subplot(111, projection='polar')
    ax.set_theta_zero_location('N')
    ax.set_theta_direction(-1)
    ax.grid(True)
    ax.set_title("RPLIDAR — latest revolution")

    scatter = ax.scatter([], [], s=4)

    # keep r-limit steady-ish to avoid redraw cost thrash
    rmax_seen = 1000.0

    def update_plot():
        nonlocal rmax_seen
        if reader.error:
            ax.set_title(f"LIDAR error: {reader.error}", color='red')
            fig.canvas.draw_idle()
            return
        thetas, dists = reader.get_latest()
        if thetas.size == 0:
            return
        scatter.set_offsets(np.c_[thetas, dists])

        # lazily relax rlim upward; shrink only occasionally
        rmax_now = float(dists.max())
        if rmax_now > rmax_seen * 0.95:
            rmax_seen = max(1000.0, rmax_now)
            ax.set_rlim(0, int(np.ceil(rmax_seen / 500.0) * 500))

    timer = fig.canvas.new_timer(interval=33)  # ~30 FPS
    timer.add_callback(lambda: (update_plot(), fig.canvas.draw_idle()))
    timer.start()

    def shutdown(*_):
        timer.stop()
        reader.stop()
        plt.close('all')

    fig.canvas.mpl_connect('close_event', lambda e: shutdown())
    signal.signal(signal.SIGINT, lambda s, f: shutdown())

    try:
        while plt.get_fignums():
            plt.pause(0.05)
    finally:
        reader.stop()
        reader.join(timeout=2.0)

if __name__ == "__main__":
    main()