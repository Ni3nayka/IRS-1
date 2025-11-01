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
http://192.168.43.56:8080/?folder=/home/ubuntu/rc
python3 client.py /dev/ttyUSB0 192.168.43.192
'''

from arduinoDriver import ArduinoDriver
from videoHttpStreamer import VideoHttpStreamer
# from cameraDriver import Camera
from time import sleep

arduino = ArduinoDriver("/dev/ttyUSB0") # "/dev/ttyACM0") # дописать отключение порта arduino.start()

from lidar_client import Lidar
lidar = Lidar()

def main():

    def arduino_wait():
        while(not arduino.LastCommandIsEnd()): pass

    arduino.getRobotData()
    print("battery (raspberry):", arduino.voltage[1])
    print("battery (arduino):", arduino.voltage[0])

    lidar.print()

    # стартуем, не прям со старта, а сразу со смещением
    arduino.runMotor(3,80)
    sleep(1)
    arduino.RunForward(120, wait_end=True)
    arduino_wait()
    arduino.runMotor(3,0)
    # в теории прочистили половину 1ой трубы
    arduino.TurnRight(90+lidar.wall_angles[3], wait_end=True)
    arduino.RunForward(50-lidar.wall_dist[2], wait_end=True)

    lidar.print()

if __name__=="__main__":
    try: main()
    except Exception as e: print(e)
    pass
