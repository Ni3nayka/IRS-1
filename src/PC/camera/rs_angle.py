import pyrealsense2 as rs
import numpy as np
import cv2
import math
import random

def deproject_depth_to_points(depth_frame, intrinsics, stride=4):
    h, w = depth_frame.shape
    points = []
    for v in range(0, h, stride):
        for u in range(0, w, stride):
            z = depth_frame[v, u]
            if z <= 0 or np.isnan(z):
                continue
            x = (u - intrinsics.ppx) * z / intrinsics.fx
            y = (v - intrinsics.ppy) * z / intrinsics.fy
            points.append((x, y, z))
    return np.array(points, dtype=np.float32)

def ransac_plane(points, n_iter=150, dist_thresh=0.03):
    if len(points) < 3:
        return None, None
    best_inliers = 0
    best_n, best_d = None, None
    N = len(points)
    for _ in range(n_iter):
        idx = random.sample(range(N), 3)
        a, b, c = points[idx]
        n = np.cross(b - a, c - a)
        norm = np.linalg.norm(n)
        if norm < 1e-6:
            continue
        n /= norm
        d = -np.dot(n, a)
        dist = np.abs(points @ n + d)
        inliers = np.count_nonzero(dist < dist_thresh)
        if inliers > best_inliers:
            best_inliers = inliers
            best_n, best_d = n, d
    if best_n is None:
        return None, None
    dist = np.abs(points @ best_n + best_d)
    inlier_pts = points[dist < dist_thresh]
    centroid = inlier_pts.mean(axis=0)
    _, _, Vt = np.linalg.svd(inlier_pts - centroid)
    n_refined = Vt[-1]
    d_refined = -np.dot(n_refined, centroid)
    return n_refined / np.linalg.norm(n_refined), d_refined

def unwrap_angle(prev, current):
    diff = current - prev
    if diff > 180:
        current -= 360
    elif diff < -180:
        current += 360
    return current

def estimate_yaw(normal, prev_yaw=None, alpha=0.2):
    nx, ny, nz = normal
    raw_yaw = math.degrees(math.atan2(nx, nz))

    if prev_yaw is None:
        return raw_yaw, raw_yaw, raw_yaw
        
    yaw_unwrapped = unwrap_angle(prev_yaw, raw_yaw)
    yaw_smooth = alpha * yaw_unwrapped + (1 - alpha) * prev_yaw
    return raw_yaw, yaw_smooth, yaw_smooth


def get_raw_yaw_from_pipeline(pipeline, prev_yaw=None, count=5, alpha=0.2):
    frames = pipeline.wait_for_frames()
    depth_frame = frames.get_depth_frame()
    if not depth_frame:
        return None

    profile = pipeline.get_active_profile()
    intr = profile.get_stream(rs.stream.depth).as_video_stream_profile().get_intrinsics()
    depth_sensor = profile.get_device().first_depth_sensor()
    depth_scale = depth_sensor.get_depth_scale()

    depth = np.asanyarray(depth_frame.get_data()).astype(np.float32) * depth_scale
    h, w = depth.shape

    x0, y0, ww, hh = int(w*0.3) - 40, int(h*0.3) + 10, int(w*0.4), int(h*0.4)
    roi = depth[y0:y0+hh, x0:x0+ww]

    intr_roi = rs.intrinsics()
    intr_roi.width = ww
    intr_roi.height = hh
    intr_roi.ppx = intr.ppx - x0
    intr_roi.ppy = intr.ppy - y0
    intr_roi.fx = intr.fx
    intr_roi.fy = intr.fy
    intr_roi.model = intr.model
    intr_roi.coeffs = intr.coeffs

    pts = deproject_depth_to_points(roi, intr_roi, stride=4)
    n, d = ransac_plane(pts)
    if n is None:
        return None

    n /= np.linalg.norm(n)
    yaws = []
    for i in range(count):
        raw_yaw, yaw_smooth, prev_yaw = estimate_yaw(n, prev_yaw, alpha)
        if -60 < raw_yaw < 60:
            yaws.append(raw_yaw)

    return -1 * np.median(np.array(yaws)), roi[0][0
    ]


def get_obj(pipeline): 
    frames = pipeline.wait_for_frames()
    depth_frame = frames.get_depth_frame()
    profile = pipeline.get_active_profile()
    intr = profile.get_stream(rs.stream.depth).as_video_stream_profile().get_intrinsics()
    depth_sensor = profile.get_device().first_depth_sensor()
    depth_scale = depth_sensor.get_depth_scale()

    depth = np.asanyarray(depth_frame.get_data()).astype(np.float32) * depth_scale
    h, w = depth.shape

    x0, y0, ww, hh = int(w*0.3) - 40, int(h*0.3) + 10, int(w*0.4), int(h*0.4)
    roi = depth[y0:y0+hh, x0:x0+ww]
    return [roi[0][0], roi[1][0], roi[2][0]]

def get_snow(pipeline): 
    frames = pipeline.wait_for_frames()
    depth_frame = frames.get_depth_frame()
    profile = pipeline.get_active_profile()
    intr = profile.get_stream(rs.stream.depth).as_video_stream_profile().get_intrinsics()
    depth_sensor = profile.get_device().first_depth_sensor()
    depth_scale = depth_sensor.get_depth_scale()

    depth = np.asanyarray(depth_frame.get_data()).astype(np.float32) * depth_scale
    h, w = depth.shape

    x0, y0, ww, hh = int(w*0.3) - 40, int(h*0.3) + 10, int(w*0.4), int(h*0.4)
    roi = depth[y0:y0+hh, x0:x0+ww]
    return [roi[40][0], roi[41][0], roi[42][0]]

def get_warn(pipeline): 
    frames = pipeline.wait_for_frames()
    depth_frame = frames.get_depth_frame()
    profile = pipeline.get_active_profile()
    intr = profile.get_stream(rs.stream.depth).as_video_stream_profile().get_intrinsics()
    depth_sensor = profile.get_device().first_depth_sensor()
    depth_scale = depth_sensor.get_depth_scale()

    depth = np.asanyarray(depth_frame.get_data()).astype(np.float32) * depth_scale
    h, w = depth.shape

    x0, y0, ww, hh = int(w*0.3) - 40, int(h*0.3) + 10, int(w*0.4), int(h*0.4)
    roi = depth[y0:y0+hh, x0:x0+ww]
    return roi[30][0] <1