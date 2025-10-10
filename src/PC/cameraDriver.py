"""
code write for project:
https://github.com/Ni3nayka/IRS-1/

author 1: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
author 2: Zhdanov 
write:  July 2025
modify: October 2025

code write with deepseek
"""

import pyrealsense2 as rs
from camera.rs_angle import *

pipeline = rs.pipeline()

class Camera:
    

    def __init__(self):
        self.config = rs.config()
        self.config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)
        pipeline.start(self.config)

    def getWall(self): # [Angle Distance]
        raw_yaw, dist = get_raw_yaw_from_pipeline(pipeline, count=10)
        return raw_yaw, dist

    def getFrame(self):
        frames = pipeline.wait_for_frames()
        color_frame = frames.get_color_frame()
        if not color_frame: return
        depth_frame = frames.get_depth_frame()
        depth_image = np.asanyarray(depth_frame.get_data())
        color_image = np.asanyarray(color_frame.get_data())
        return [color_image, depth_image] 

    def detectBigObject(self, fast=False): # [[distanse, angle, angle]]
        return get_obj(pipeline)
        color_frame = frames.get_color_frame()
        # if not color_frame: 
        depth_frame = frames.get_depth_frame()

        depth_image = np.asanyarray(depth_frame.get_data())
        color_image = np.asanyarray(color_frame.get_data())
        depth_m = depth_image * depth_scale

        mask = (depth_m > floor_height) & (depth_m < max_distance)
        mask = mask.astype(np.uint8) * 255
        roi_start = 100  # skip bottom 100 pixels (floor)
        mask[:roi_start, :] = 0

        kernel = np.ones((5,5), np.uint8)
        mask_clean = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
        mask_clean = cv2.morphologyEx(mask_clean, cv2.MORPH_CLOSE, kernel)

        contours, _ = cv2.findContours(mask_clean, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if fast:
            if len(contours) > 0: return 1
            else: return 0
        obj = []
        for cnt in contours:
            
            area = cv2.contourArea(cnt)
            if area > 500:
                x, y, w, h = cv2.boundingRect(cnt)
                X, Y, Z = rs.rs2_deproject_pixel_to_point(self.depth_intrin, [x, y], z)

                az = math.degrees(math.atan((x - cx) / fx))
                el = math.degrees(math.atan((y - cy) / fy))

                pixel_size = (x2 - x1) * (y2 - y1)

                obj.append([Z, az, el])
        return cnt

    def detectSnow(self): # [[distanse, angle, angle]]

        frames = pipeline.wait_for_frames()
        color_frame = frames.get_color_frame()
        return get_snow(pipeline) 

        color_image = np.asanyarray(color_frame.get_data())
        output_frame, mask, circles = detect_white_balls(color_image)

        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        lower_white = np.array([0, 0, 200])
        upper_white = np.array([180, 50, 255])
        
        mask = cv2.inRange(hsv, lower_white, upper_white)

        kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5))
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel, iterations=2)
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel, iterations=2)

        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        detected_circles = []

        for cnt in contours:
            ((x, y), radius) = cv2.minEnclosingCircle(cnt)
            if radius > 5:


                X, Y, Z = rs.rs2_deproject_pixel_to_point(self.depth_intrin, [x, y], z)

                az = math.degrees(math.atan((x - cx) / fx))
                el = math.degrees(math.atan((y - cy) / fy))

                pixel_size = (x2 - x1) * (y2 - y1)

                detected_circles.append([Z, az, el])

        return detected_circles


    def detectWarningObject(self): # проверка на наличие объекта прямо перед нами, чтобы не врезаться (fast method)
        return get_warn(pipeline)
        
    def close(self):
        pass