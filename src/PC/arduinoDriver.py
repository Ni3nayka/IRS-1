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
            return S[2:-5]
        return ''

    def now_read(self):
        if not self.enable:
            return 0
        S = self.ser.readline()
        return self._clean_serial_data(S)

    def write(self, S):
        if not self.enable:
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
        if not self.enable or not self.mas:
            return 0
        return self.mas.pop(0)

    def wait_read(self):
        if not self.enable:
            return 0
        while not self.mas:
            pass
        return self.mas.pop(0)

class ArduinoDriver(arduino_usb):
    def runMotor(self, left_speed, right_speed):
        # Управление моторами. left_speed и right_speed — значения скоростей (например, -100..100)
        left_speed = max(-100, min(100, int(left_speed)))
        right_speed = max(-100, min(100, int(right_speed)))
        cmd = f"m {left_speed} {right_speed}"
        print(cmd)
        self.write(cmd)

    def RunServo(self, number, angle):
        # Управление сервоприводом. number — номер серво, angle — угол (0..180)
        angle = max(0, min(180, int(angle)))
        cmd = f"s {number} {angle}"
        self.write(cmd)

    def RunEnc(self, forward, right):
        # движемся по энкодерам (читай мануал в коде ардуино)
        cmd = f"E {forward} {right}"
        self.write(cmd)

if __name__ == "__main__":
    from time import sleep
    # arduino = ArduinoDriver('/dev/ttyUSB0')
    arduino = ArduinoDriver('/dev/ttyACM0')
    arduino.start()
    sleep(2) # Иначе ардуинка не успевает включиться
    arduino.runMotor(90,90)
    sleep(1)
    arduino.runMotor(0,0)
    arduino.enable = False
    # while (0):
    #     if (arduino.available()):
    #         print(arduino.read())
else:
    pass
