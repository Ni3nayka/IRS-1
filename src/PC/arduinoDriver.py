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

Commands:
from raspberry_arduino_USB import *
serial_read() # return String input
serial_write(String output)

pip install pyserial
'''

print("$ import <python_arduino_USB_plus>")

import serial
from threading import Thread

# поиск порта для разбери пай: ввести в терминал    ls /dev/tty*
# Затем найдите строку с /dev/ttyACM0 или что-то вроде /dev/ttyACM1 и т.д. Проверьте ACM с любым числом 0,1,2 и т.д.

#ser=serial.Serial('com6',9600)
#ser=serial.Serial("/dev/ttyUSB0",9600)
#ser.baudrate=9600

def python_arduino_USB_plus_version(S=0):
    version = "1.2"
    if (S==0):
        return version
    if (str(S)==version):
        return 1
    else:
        print("<python_arduino_USB_plus> have more version!!!")
        return 0


#import os
#x=os.system("ls /dev/ttyUSB0")
#if x==0:
#    print "connected"
#else:
#    print "disconnected"

class arduino_usb(Thread):
    
    def __init__(self,com): # com = 'com6' "/dev/ttyUSB0"
        Thread.__init__(self)
        self.ser = 0 # serial.Serial(com,9600)
        self.mas = []
        self.enable = 1
        try:
            self.ser = serial.Serial(com,9600)
        except serial.serialutil.SerialException:
            self.enable = 0
            print("ERROR <python_arduino_USB_plus>: no USB device:", com)

    def port(self):
        return self.enable
    
    def now_read(self):
        if (not self.enable): return 0
        S = self.ser.readline()
        # b'1\r\n' => 1
        #print(S, " ", end='')
        # отрезаем "мусор"
        S = str(S)
        S = list(S)  # str => mas
        del S[0]
        del S[0]
        del S[len(S)-1]
        del S[len(S)-1]
        del S[len(S)-1]
        del S[len(S)-1]
        del S[len(S)-1]
        S = ''.join(S) # mas => str
        #print(S)
        return S

    def write(self,S):
        if (not self.enable): return 0
        #ser.write(b'hello, arduino!!!\n') # \r
        S = str(S)
        S1 = bytearray(b'')
        i = 0
        while (i<len(S)):
            S1.append(ord(S[i]))
            i += 1
        S1.append(ord('\n'))
        #print(bytes(S1))
        self.ser.write(S1)
    
    def run(self): # Запуск потока
        if (not self.enable): return 0
        while (1):
            S = self.ser.readline()
            # b'1\r\n' => 1
            #print(S, " ", end='')
            # отрезаем "мусор"
            S = str(S)
            S = list(S)  # str => mas
            del S[0]
            del S[0]
            del S[len(S)-1]
            del S[len(S)-1]
            del S[len(S)-1]
            del S[len(S)-1]
            del S[len(S)-1]
            S = ''.join(S) # mas => str
            #print(S)
            #print(S)
            self.mas.append(S)
            
    def available(self):
        if (not self.enable): return 0
        if (len(self.mas)>0):
            return 1
        else:
            return 0
        
    def read(self):
        if (not self.enable): return 0
        if (len(self.mas)>0):
            S = self.mas[0]
            del self.mas[0]
            return S
        else:
            return 0
        
    def wait_read(self):
        if (not self.enable): return 0
        while (len(self.mas)==0): pass
        S = self.mas[0]
        del self.mas[0]
        return S


if __name__ == "__main__":
    #main()
    arduino = arduino_usb('/dev/ttyUSB0')
    arduino.start()
    while (0):
        if (arduino.available()):
            print(arduino.read())
    #print(arduino.read())
    #print(arduino.read())
    #print(arduino.read())
else:
    # приветствие в моей проге ардуино
    #serial_read()
    #print("$ ROBOT:", serial_read())
    #serial_write("@+1+1")
    #serial_write("+")
    #print(str(serial_read()))
    pass
