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
from time import sleep
from arduinoDriver import ArduinoDriver
arduino = ArduinoDriver("/dev/ttyUSB0") # "/dev/ttyACM0") # дописать отключение порта arduino.start()

def main():

    def arduino_wait():
        while(not arduino.LastCommandIsEnd()): pass

    arduino.getRobotData()
    print("battery (raspberry):", arduino.voltage[1])
    print("battery (arduino):", arduino.voltage[0])

    # ровняемся на первую трубу
    arduino.TurnLeft(90, wait_end=True)
    arduino.RunForward(25, wait_end=True)
    arduino.TurnRight(97, wait_end=True)

    # стартуем, не прям со старта, а сразу со смещением
    arduino.runMotor(3,100)
    sleep(1)
    arduino.RunForward(135, wait_end=True)
    # arduino.runMotor(3,0)
    # в теории прочистили половину 1ой трубы, объезжаем ее
    arduino.TurnRight(85, wait_end=True)
    arduino.RunForward(43, wait_end=True)
    # чистим с другой стороны
    sleep(1)
    arduino.TurnRight(90, wait_end=True)
    arduino.runMotor(3,100)
    arduino.RunForward(135, wait_end=True)
    arduino.runMotor(3,0)
    # становимся на старт
    arduino.TurnRight(90, wait_end=True)
    arduino.RunForward(38, wait_end=True)

if __name__=="__main__":
    main()
    # try: main()
    # except Exception as e: print(e)
    pass
