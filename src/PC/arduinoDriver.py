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

code have big update for robofinist 2025 - kubok RTC - vishaya liga 
'''

import serial
from time import sleep

class arduino_usb:
    def __init__(self, com):
        self.serial_device = None
        self.buffer = []
        self.enable = True
        try:
            self.serial_device = serial.Serial(com, 9600)
            sleep(5) # чтобы ардуинка успела загрузиться
        except serial.serialutil.SerialException:
            self.enable = False
            print(f"ERROR <python_arduino_USB_plus>: no USB device: {com}")

    def write(self, S, ignore_enable=0):
        if not self.enable and not ignore_enable:
            return 0
        S = str(S) + '\n'
        self.serial_device.write(S.encode('utf-8'))

    def _updateForRead(self):
        sleep(0.1)
        data = self.serial_device.read_all()
        # print(data)
        if not data: return
        # декодируем в строку (безопасно, заменяем нечитаемые байты)
        s = data.decode('utf-8', errors='replace')
        # получаем список строк без пустых элементов
        lines = [line for line in s.strip().splitlines() if line]
        # print("raw string:", s)   # вся строка с \r\n
        # print("lines:", lines)
        self.buffer += lines
        # print(self.buffer)

    def available(self):
        if not self.enable:
            return 0
        self._updateForRead()
        return len(self.buffer)

    def read(self):
        if self.available() == 0:
            return ""
        # Удаляем пустые списки в начале буфера (на всякий случай)
        while self.buffer and not self.buffer[0]:
            self.buffer.pop(0)
        if not self.buffer:
            return ""
        # Берём первый элемент из первого списка и удаляем его
        elem = self.buffer.pop(0)
        return elem

    def wait_read(self):
        if not self.enable:
            return 0
        while self.available() == 0: pass
        return self.read()

class ArduinoDriver(arduino_usb):

    def __init__(self, com):
        super().__init__(com)
        self.waiting_for_a_response = False
        self.gy25 = [0,0,0,0]
        self.enc = [0,0]
        self.voltage = [0,0]

    def _constrain(self, value, min_value, max_value):
        return max(min_value, min(max_value, int(value)))

    def runMotor(self, number, speed):
        # Управление моторами. left_speed и right_speed — значения скоростей (например, -100..100)
        speed = self._constrain(speed, -100, 100)
        number = self._constrain(number, 1, 4)
        cmd = f"m {number} {speed}"
        # print(cmd)
        self.write(cmd)

    def runMotor2(self, left, right):
        left = self._constrain(left, -100, 100)
        right = self._constrain(right, -100, 100)
        cmd = f"M {left} {right}"
        # print(cmd)
        self.write(cmd)

    def RunServo(self, number, angle):
        # Управление сервоприводом. number — номер серво, angle — угол (0..180)
        angle = self._constrain(angle, 0, 180)
        cmd = f"s {number} {angle}"
        self.write(cmd)

    def RunForward(self, forward, wait_end=False):
        self.buffer.clear()
        self.waiting_for_a_response = True
        cmd = f"F {forward}"
        self.write(cmd)
        if wait_end: 
            while(not self.LastCommandIsEnd()): pass

    def TurnLeft(self, left, wait_end=False):
        self.buffer.clear()
        self.waiting_for_a_response = True
        cmd = f"L {left}"
        self.write(cmd)
        if wait_end: 
            while(not self.LastCommandIsEnd()): pass

    def TurnRight(self, right, wait_end=False):
        self.buffer.clear()
        self.waiting_for_a_response = True
        cmd = f"R {right}"
        self.write(cmd)
        if wait_end: 
            while(not self.LastCommandIsEnd()): pass

    def getRobotData(self):
        '''
        gy25 (angle): x y z z_strafe
        enc: left right
        voltage: arduino(1), raspberry(2)
        '''
        def translater(array,line,line_test,count_parametrs,parameters_is_float):
            if line.startswith(line_test):
                parts = line.split()
                if len(parts) == count_parametrs+1:
                    try:
                        if parameters_is_float:
                            array = [float(num) for num in parts[1:]]
                        else:
                            array = [int(num) for num in parts[1:]]
                    except ValueError:
                        pass
            return array

        if not self.enable:
            return 0
        self.available() # обновляем доступные данные
        self.buffer.clear()
        self.write("g")
        while self.available()<3: pass # print(self.available())
        # print(self.read())
        # print(self.read())
        # print(self.read())
        self.gy25 = translater(self.gy25,self.read(),"GY25:",4,False)
        self.enc = translater(self.enc,self.read(),"ENC:",2,False)
        self.voltage = translater(self.voltage,self.read(),"VOLTAGE:",2,True)

    def LastCommandIsEnd(self):
        if not self.waiting_for_a_response: return True
        self.available() # обновляем доступные данные
        found = any("END COMMAND:" in str(line).upper() for line in self.buffer)
        self.waiting_for_a_response = False if found else True
        return not self.waiting_for_a_response

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
    # arduino.runMotor(1,10)
    # sleep(1)
    # arduino.runMotor(1,0)
    # sleep(5)
    # arduino.runMotor2(20,20)
    # sleep(3)
    # arduino.runMotor(0,0)
    # sleep(3)
    # arduino.RunForward(30)
    # sleep(10)
    # arduino.TurnLeft(90)
    # sleep(10)
    # arduino.TurnRight(90)
    # sleep(10)
    # print("Wait")
    # while 1:
    #     print(arduino.ser.readline().decode("utf-8"))
    # arduino.enable = False

    # Test 3
    arduino.getRobotData()
    print(arduino.gy25)
    print(arduino.enc)
    print(arduino.voltage)
    print()
    sleep(2)
    
    while 1:
        arduino.RunForward(100)
        a = False
        while not a:
            a = arduino.LastCommandIsEnd()
            print(a)
            sleep(0.5)
        print()
        sleep(0.5)
    