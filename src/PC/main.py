'''
ssh ni3nayka@10.91.215.9
http://10.91.215.9:8080
http://10.91.215.9:8080/?folder=/home/ni3nayka/IRS

открыть консольку в VScodeServer:
ctrl+(shift)+`

права порту, чтобы к нему мы имели доступ:
sudo chmod 666 /dev/ttyACM0
'''

from arduinoDriver import ArduinoDriver
from videoHttpStreamer import VideoHttpStreamer
from cameraDriver import Camera
from time import sleep

arduino = ArduinoDriver("/dev/ttyACM0") # дописать отключение портаarduino.start()
sleep(2) # Иначе ардуинка не успевает включиться
# arduino.runMotor(-50,-50)
# sleep(2)
arduino.runMotor(0,0)

# def firstTest():
#     pass
# if __name__ == "__main__":
#     firstTest()

# CAMERA TEST

camera = Camera()
wall = camera.getWall()
print(wall[0], wall[1])

print(camera.detectBigObject())
print(camera.detectSnow())
print(camera.detectWarningObject())