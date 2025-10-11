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
'''

from arduinoDriver import ArduinoDriver
from videoHttpStreamer import VideoHttpStreamer
from cameraDriver import Camera
from time import sleep

arduino = ArduinoDriver("/dev/ttyUSB1") # "/dev/ttyACM0") # дописать отключение порта arduino.start()
sleep(5) # Иначе ардуинка не успевает включиться

def main():

    # arduino.runMotor(1,10)
    # sleep(1)
    # arduino.runMotor(1,0)


    arduino.RunServo(1,180)
    sleep(3)
    arduino.runMotor(3,40)
    arduino.RunForward(120)
    while(not arduino.CheckEnc()): pass
    arduino.runMotor(3,0)
    arduino.RunServo(1,90)
    arduino.TurnRight(180)
    arduino.RunForward(120)

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
    arduino.RunServo(1,90)
    arduino.RunServo(2,90)
    arduino.runMotor(1,0)
    arduino.runMotor(2,0)
    arduino.runMotor(3,0)
    arduino.runMotor(4,0)
    arduino.runMotor(4,0)
    arduino.enable = 0
    arduino.write("e 1 1",ignore_enable=1)
