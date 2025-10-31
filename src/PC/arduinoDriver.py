'''
This program writing Bakay Egor, Moscow, Russia, 2020
This script is part of the project "PROMETHEUS"
https://www.youtube.com/playlist?list=PL24VxeCr7LD3qzQenzNS4zYKe5VYeBOxw

Info:
https://arduinoplus.ru/podkluchenie-raspberry-arduino/
https://microkontroller.ru/arduino-projects/ispolzovanie-yazyka-programmirovaniya-python-vmeste-s-arduino/
https://pythonworld.ru/tipy-dannyx-v-python/bajty-bytes-i-bytearray.html
https://zen.yandex.ru/media/id/5c1a663bb6a0da00aac86ae4/python-vstroennye-funkcii-chr--ord--30-5ce56aec25bf1600b3965727
https://python-scripts.com/threading

pip install pyserial

code writing with copilot
'''

import serial
from threading import Thread

class arduino_usb(Thread):
    def __init__(self, com):
        super().__init__()
        self.ser = None
        self.mas = []
        self.enable = True
        try:
            self.ser = serial.Serial(com, 9600)
        except serial.serialutil.SerialException:
            self.enable = False
            print(f"ERROR <python_arduino_USB_plus>: no USB device: {com}")

    def port(self):
        return self.enable

    @staticmethod
    def _clean_serial_data(S):
        # Удаляет первые 2 и последние 5 символов
        S = str(S)
        if len(S) > 7:
            # return S[2:-5]
            return S
        return ''

    def now_read(self):
        if not self.enable:
            return 0
        S = self.ser.readline()
        return self._clean_serial_data(S)

    def flush(self):
        self.ser.reset_input_buffer()
        self.ser.read_all()

    def write(self, S, ignore_enable=0):
        if not self.enable and not ignore_enable:
            return 0
        S = str(S) + '\n'
        self.ser.write(S.encode('utf-8'))

    def run(self):
        while self.enable:
            S = self.ser.readline()
            cleaned = self._clean_serial_data(S)
            self.mas.append(cleaned)
            print(cleaned)
        return 0

    def available(self):
        if not self.enable:
            return 0
        return int(bool(self.mas))

    def read(self):
        if not self.enable or len(self.mas) == 0:
            return 0
        return self.mas.pop(0)

    def wait_read(self):
        if not self.enable:
            return 0
        while not self.mas:
            pass
        return self.mas.pop(0)

class ArduinoDriver(arduino_usb):
    # def runMotor(self, left_speed, right_speed):
    #     # Управление моторами. left_speed и right_speed — значения скоростей (например, -100..100)
    #     left_speed = max(-100, min(100, int(left_speed)))
    #     right_speed = max(-100, min(100, int(right_speed)))
    #     cmd = f"m {left_speed} {right_speed}"
    #     print(cmd)
    #     self.write(cmd)
    def runMotor(self, number, speed):
        # Управление моторами. left_speed и right_speed — значения скоростей (например, -100..100)
        speed = max(-100, min(100, int(speed)))
        number = max(1, min(4, int(number)))
        cmd = f"m {number} {speed}"
        print(cmd)
        self.write(cmd)

    def runMotor2(self, left, right):
        left = max(-100, min(100, int(left)))
        right = max(-100, min(100, int(right)))
        cmd = f"M {left} {right}"
        print(cmd)
        self.write(cmd)

    def RunServo(self, number, angle):
        # Управление сервоприводом. number — номер серво, angle — угол (0..180)
        angle = max(0, min(180, int(angle)))
        cmd = f"s {number} {angle}"
        self.write(cmd)

    def RunForward(self, forward):
        # движемся по энкодерам (читай мануал в коде ардуино)
        self.flush()
        cmd = f"F {forward}"
        self.write(cmd)

    def TurnLeft(self, left):
        self.flush()
        cmd = f"L {left}"
        self.write(cmd)

    def TurnRight(self, right):
        self.flush()
        cmd = f"R {right}"
        self.write(cmd)

    def getRobotData():
        pass

    def CheckEnc(self):
        msg = self.ser.readline().decode("utf-8")
        print(msg)
        return "&" in msg



if __name__ == "__main__":
    from time import sleep
    # arduino = ArduinoDriver('/dev/ttyUSB0')
    '''
    arduino = ArduinoDriver('/dev/ttyACM0')
    arduino.start()
    sleep(2) # Иначе ардуинка не успевает включиться
    arduino.runMotor(90,90)
    sleep(1)
    arduino.runMotor(0,0)
    arduino.enable = False
    '''
    # while (0):
    #     if (arduino.available()):
    #         print(arduino.read())

    # Test 2
    arduino = ArduinoDriver('/dev/ttyUSB0')
    sleep(5)
    arduino.runMotor(1,10)
    sleep(1)
    arduino.runMotor(1,0)
    sleep(5)
    arduino.runMotor2(20,20)
    sleep(3)
    arduino.runMotor(0,0)
    sleep(3)
    arduino.RunForward(30)
    sleep(10)
    arduino.TurnLeft(90)
    sleep(10)
    arduino.TurnRight(90)
    sleep(10)
    print("Wait")
    while 1:
        print(arduino.ser.readline().decode("utf-8"))
    

    arduino.enable = False
