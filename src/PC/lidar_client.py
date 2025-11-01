import socket
import json
import numpy as np
from sklearn.linear_model import RANSACRegressor
import threading
import queue
import time



def polar_to_cartesian(angles, distances):
    x = distances * np.cos(angles)
    y = distances * np.sin(angles)
    return x, y

def detect_walls_ransac_with_points(x, y, max_lines=4, threshold=50, min_points=10):
    """Detect walls using RANSAC and return wall points"""
    from sklearn.linear_model import RANSACRegressor
    
    walls = []  # Each element: (m, b, inlier_indices, inlier_points)
    remaining_indices = np.arange(len(x))
    
    for _ in range(max_lines):
        if len(remaining_indices) < min_points:
            break
        
        model = RANSACRegressor(residual_threshold=threshold, random_state=42)
        model.fit(x[remaining_indices].reshape(-1, 1), y[remaining_indices])
        inlier_mask = model.inlier_mask_
        
        if np.sum(inlier_mask) < min_points:
            break
            
        # Get the actual inlier points
        wall_indices = remaining_indices[inlier_mask]
        wall_points = np.column_stack((x[wall_indices], y[wall_indices]))
        
        m = model.estimator_.coef_[0]
        b = model.estimator_.intercept_
        
        walls.append((m, b, wall_indices, wall_points))
        
        # Remove inliers for next iteration
        remaining_indices = remaining_indices[~inlier_mask]
    
    return walls
    
import numpy as np
import math

class YawIntegrator:
    def __init__(self, initial_yaw=0.0, alpha=0.3, max_yaw_change=30.0):
        """
        Yaw integrator that maintains state and integrates new measurements
        
        Args:
            initial_yaw: Initial yaw in degrees
            alpha: Smoothing factor (0.0-1.0), higher = more trust in new measurements
            max_yaw_change: Maximum allowed yaw change per update in degrees
        """
        self.current_yaw = initial_yaw  # degrees
        self.alpha = alpha
        self.max_yaw_change = max_yaw_change
        self.initialized = False
        
    def update_from_walls(self, walls, confidence=1.0):
        """
        Update yaw estimate based on wall detection
        
        Args:
            walls: List of (m, b) wall line equations
            confidence: Confidence of the new measurement (0.0-1.0)
        
        Returns:
            new_yaw: Updated yaw in degrees
        """
        if len(walls) < 2:
            return self.current_yaw
            
        # Calculate yaw from walls
        new_yaw = self._calculate_yaw_from_walls(walls)
        
        # For first measurement, just set the yaw
        if not self.initialized:
            self.current_yaw = new_yaw
            self.initialized = True
            return self.current_yaw
        
        # Apply smoothing with confidence weighting
        effective_alpha = self.alpha * confidence
        
        # Handle angle wrapping (ensure shortest path)
        current_rad = math.radians(self.current_yaw)
        new_rad = math.radians(new_yaw)
        
        # Find shortest angular difference
        diff = new_rad - current_rad
        while diff > math.pi:
            diff -= 2 * math.pi
        while diff < -math.pi:
            diff += 2 * math.pi
            
        # Limit maximum change
        max_change_rad = math.radians(self.max_yaw_change)
        diff = np.clip(diff, -max_change_rad, max_change_rad)
        
        # Apply smoothing
        updated_rad = current_rad + effective_alpha * diff
        self.current_yaw = math.degrees(updated_rad) % 360
        
        return self.current_yaw
    
    def _calculate_yaw_from_walls(self, walls):
        """Calculate yaw from wall orientations"""
        wall_angles = []
        
        for m, b in walls:
            angle_rad = math.atan(m)  # Angle of wall normal
            wall_angles.append(angle_rad)
        
        # Find the angle that best aligns walls with cardinal directions
        best_yaw = 0
        best_error = float('inf')
        
        # Try different possible alignments (every 1 degree for precision)
        for test_yaw_deg in np.arange(0, 180, 1):
            test_yaw_rad = math.radians(test_yaw_deg)
            error = 0
            
            for wall_angle in wall_angles:
                # How far is this wall from being aligned with 0°, 90°, etc.?
                aligned_angle = (wall_angle - test_yaw_rad) % (math.pi/2)
                if aligned_angle > math.pi/4:
                    aligned_angle = math.pi/2 - aligned_angle
                error += aligned_angle
            
            if error < best_error:
                best_error = error
                best_yaw = test_yaw_deg
        
        return best_yaw
    
    def get_yaw(self):
        """Get current yaw estimate in degrees"""
        return self.current_yaw
    
    def reset(self, yaw=0.0):
        """Reset integrator to specified yaw"""
        self.current_yaw = yaw
        self.initialized = True

# Complete pose estimation with yaw integration
class PoseEstimator:
    def __init__(self, initial_yaw=0.0, initial_pos=(0, 0)):
        """
        Complete pose estimator with yaw integration
        
        Args:
            initial_yaw: Initial yaw in degrees
            initial_pos: Initial position (x, y) in mm
        """
        self.yaw_integrator = YawIntegrator(initial_yaw)
        self.position = np.array(initial_pos, dtype=float)
        self.initialized = False
        
    def update_pose(self, walls, position_confidence=1.0):
        """
        Update both position and yaw based on wall data
        
        Args:
            walls: List of (m, b) wall line equations
            position_confidence: Confidence in position measurement (0.0-1.0)
        
        Returns:
            dict: {'x', 'y', 'yaw', 'confidence'}
        """
        if len(walls) < 2:
            return self._get_current_pose(0.0)
        
        # Calculate confidence based on number of walls and consistency
        confidence = self._calculate_confidence(walls)
        
        # Update yaw with wall-based estimation
        yaw = self.yaw_integrator.update_from_walls(walls, confidence)
        
        # Update position (simplified - you can enhance this)
        position = self._estimate_position(walls, yaw, position_confidence)
        if position is not None:
            self.position = position
        
        return self._get_current_pose(confidence)
    
    def _estimate_position(self, walls, yaw, confidence):
        """Estimate position from walls and known yaw"""
        if len(walls) < 2:
            return None
            
        # Simple method: Use intersections of perpendicular walls
        wall_angles = [math.atan(m) for m, b in walls]
        yaw_rad = math.radians(yaw)
        
        # Find walls that are approximately perpendicular to each other
        perpendicular_pairs = []
        for i, angle1 in enumerate(wall_angles):
            for j, angle2 in enumerate(wall_angles[i+1:], i+1):
                angle_diff = abs(angle1 - angle2) % math.pi
                if abs(angle_diff - math.pi/2) < math.radians(15):  # 15° tolerance
                    perpendicular_pairs.append((i, j))
        
        if perpendicular_pairs:
            # Use the best perpendicular pair
            i, j = perpendicular_pairs[0]
            m1, b1 = walls[i]
            m2, b2 = walls[j]
            
            # Calculate intersection point (room corner)
            if abs(m1 - m2) > 1e-10:
                corner_x = (b2 - b1) / (m1 - m2)
                corner_y = m1 * corner_x + b1
                
                # Estimate robot position relative to corner
                # This is simplified - in practice you'd use LiDAR min distances
                estimated_pos = np.array([corner_x + 500, corner_y + 500])
                
                # Smooth position update
                if not self.initialized:
                    self.position = estimated_pos
                    self.initialized = True
                else:
                    self.position = self.position + confidence * (estimated_pos - self.position)
        
        return self.position
    
    def _calculate_confidence(self, walls):
        """Calculate confidence based on wall consistency"""
        if len(walls) < 2:
            return 0.0
        
        # Base confidence on number of walls
        base_confidence = min(1.0, len(walls) / 4.0)
        
        # Additional confidence from wall perpendicularity
        wall_angles = [math.atan(m) for m, b in walls]
        perpendicularity_score = 0.0
        
        for i, angle1 in enumerate(wall_angles):
            for j, angle2 in enumerate(wall_angles[i+1:], i+1):
                angle_diff = abs(angle1 - angle2) % math.pi
                if abs(angle_diff - math.pi/2) < math.radians(20):
                    perpendicularity_score += 0.2
        
        return min(1.0, base_confidence + perpendicularity_score)
    
    def _get_current_pose(self, confidence):
        """Get current pose as dictionary"""
        return {
            'x': self.position[0],
            'y': self.position[1],
            'yaw': self.yaw_integrator.get_yaw(),
            'confidence': confidence
        }
    
    def get_pose(self):
        """Get current pose without update"""
        return self._get_current_pose(0.0)



def wall_navigation(yaw_ptr, pose_ptr, wall_distances_ptr, wall_angles_ptr, host='localhost', port=9999):
    """Simple client with integrated yaw estimation"""
    import socket
    import json
    import time
    
    # Initialize pose estimator (assuming robot starts facing 0°)
    pose_estimator = PoseEstimator(initial_yaw=0.0, initial_pos=(0, 0))
    
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        client_socket.connect((host, port))
        print(f"Connected to LiDAR server at {host}:{port}")
        print("Yaw integration active - maintaining state across scans")
        print("Waiting for LiDAR data...\n")
        
        buffer = ""
        scan_count = 0
        
        while True:
            data = client_socket.recv(4096).decode('utf-8')
            if not data:
                break
            
            buffer += data
            lines = buffer.split('\n')
            buffer = lines[-1]
            
            for line in lines[:-1]:
                if line.strip():
                    try:
                        scan_data = json.loads(line)
                        points = scan_data['points']
                        scan_count += 1

                        
                        if len(points) >= 10:
                            angles = np.radians([a for a, d in points])
                            distances = np.array([d for a, d in points])
                            
                            valid_mask = (distances > 0) & (distances < 10000)
                            angles = angles[valid_mask]
                            distances = distances[valid_mask]

                            
                            
                            
                            if len(angles) >= 10:
                                x, y = polar_to_cartesian(angles, distances)

                                walls_with_points = detect_walls_ransac_with_points(x, y)

                                # Extract just the line equations (m, b) for pose estimation
                                if len(walls_with_points) >= 2:
                                    wall_lines = [(m, b) for m, b, indices, points in walls_with_points]

                                    # Update pose with the wall lines
                                    pose = pose_estimator.update_pose(wall_lines, position_confidence=0.8)
                                    pose_ptr[0] = pose['x']
                                    pose_ptr[1] = pose['x']
                                    yaw_ptr[0] = pose['yaw']
                                    for i, (m, b, indices, points) in enumerate(walls_with_points):
                                        wall_distances_ptr[0] = np.sqrt(points[:, 0]**2 + points[:, 1]**2).flatten().tolist()
                                        wall_angles_ptr[0] = np.degrees(np.arctan2(points[:, 1], points[:, 0])).flatten().tolist()
                                    
                                else:
                                    print("Need at least 2 walls for pose estimation")
                            
                        
                    except json.JSONDecodeError:
                        print("Invalid JSON data")
                    except Exception as e:
                        print(f"Error: {e}")
            
            time.sleep(0.1)
                        
    except KeyboardInterrupt:
        print("\nDisconnecting...")
        final_pose = pose_estimator.get_pose()
        print(f"Final estimated pose: {final_pose}")
    except Exception as e:
        print(f"Client error: {e}")
    finally:
        client_socket.close()



class Lidar:
    def __init__(self):
        self.yaw = [0] # угол абсолютной
        self.pose = [0, 0] # позиция робота на поле x y
        self.wall_dist = [[]] # расстояния до стен
        self.wall_angles = [[]] # углы до стен
        self.thr = threading.Thread(target=wall_navigation, args=(self.yaw, self.pose, self.wall_dist, self.wall_angles))
        self.thr.start()
    def print(self):
        print("угол абсолютный: ", self.yaw[0])
        print("позиция робота: ", self.pose)
        print("расстояния до стен: ", self.wall_dist)
        print("углы до стен:       ", self.wall_angles)

if __name__ == "__main__":
    yaw = [0]
    pose = [0, 0]
    wall_dist = [[]]
    wall_angles = [[]]
    thr = threading.Thread(target=wall_navigation, args=(yaw, pose, wall_dist, wall_angles))
    thr.start()

    while 1:
        print(yaw, pose)
        time.sleep(1)
