'''
ssh ni3nayka@10.91.215.9
http://10.91.215.9:8080
http://10.91.215.9:8080/?folder=/home/ni3nayka/IRS

открыть консольку в VScodeServer:
ctrl+(shift)+`

права порту, чтобы к нему мы имели доступ:
sudo chmod 666 /dev/ttyACM0
sudo chmod 666 /dev/ttyUSB0
ls /dev | grep USB

ffplay /dev/video2
'''

from arduinoDriver import ArduinoDriver
from videoHttpStreamer import VideoHttpStreamer
# from cameraDriver import Camera
from time import sleep

arduino = ArduinoDriver("/dev/ttyUSB0") # "/dev/ttyACM0") # дописать отключение порта arduino.start()

from lidar_client import Lidar
lidar = Lidar()

# while 1:
#     print(yaw, pose)
#     time.sleep(1)

# class coordinates:
#     def __init__(self,x=0,y=0,dir=0,name=None):
#         self.x = x
#         self.y = y
#         self.dir = dir # 0 - в направлении первой трубы
#         self.name = name
#     def copy(self,name=None):
#         return coordinates(self.x,self.y,self.dir,name)
#     def print(self):
#         print("coordinates", end='')
#         if self.name!=None: print(" (" + str(self.name) + ")", end='')
#         print(": x="+str(self.x)+" y="+str(self.y)+" dir="+str(self.dir))
# real_coo = coordinates(40,40,90,"real")
# target_coo = real_coo.copy("target")
# real_coo.print()
# target_coo.print()

def error():
    pass

def main():
    arduino.getRobotData()
    print("battery (raspberry):", arduino.voltage[1])
    print("battery (arduino):", arduino.voltage[0])

    # arduino.RunServo(1,180)
    # sleep(3)
    # arduino.runMotor(3,40)

    lidar.print()

    arduino.runMotor(3,80)
    sleep(1)
    arduino.RunForward(120)
    while(not arduino.LastCommandIsEnd()): pass
    arduino.runMotor(3,0)

    lidar.print()

# CAMERA TEST

# camera = Camera()
# wall = camera.getWall()
# print(wall[0], wall[1])

# print(camera.detectBigObject())
# print(camera.detectSnow())
# print(camera.detectWarningObject())

if __name__=="__main__":
    try: main()
    except Exception as e: print(e)
    # arduino.RunServo(1,90)
    # arduino.RunServo(2,90)
    # arduino.runMotor(1,0)
    # arduino.runMotor(2,0)
    # arduino.runMotor(3,0)
    # arduino.runMotor(4,0)
    # arduino.runMotor(4,0)
    # arduino.enable = 0
    # arduino.write("e 1 1",ignore_enable=1)
    pass
