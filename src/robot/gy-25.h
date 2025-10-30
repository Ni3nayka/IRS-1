/*
   arduino library for gy-25 module:
   https://github.com/Ni3nayka/gy-25

   author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
   write:  June 2024
   modify: September 2025

   ВНИМАНИЕ!!!
   Эта либа написана на коленке в самый последний момент, на основе разроаботок с предыдущей олимпиады, 
   по принципу "работает и ладно". Код - г@вно, но функционал выполняет
*/  

#pragma once

#ifndef GY25_SERIAL_BOD
#define GY25_SERIAL_BOD 115200
#endif

#ifndef GY25_STRAFE_DT
#define GY25_STRAFE_DT 0 // 1500 // 2100
#endif

#ifndef GY25_STRAFE_ANDLE
#define GY25_STRAFE_ANDLE -1 // подруливать в: 1 - вправо, -1 - влево
#endif

#ifndef GY25_SERIAL
  #if (defined(__AVR__))
  #include <SoftwareSerial.h>
  #elif (defined(ESP32)) 
  #include "SoftwareSerial.h" 
  #else
  #error "lib not supported this board"
  #endif
#endif

// SoftwareSerial mySerial(10, 11); // RX, TX

#ifdef GY25_SERIAL
class GY25 {
#else
class GY25: private SoftwareSerial {  //(): public SoftwareSerial {
#endif
  public:
    int angle[3];
    long int horizontal_angle;

    void setup() {
      GY25::begin(GY25_SERIAL_BOD);
      delay(1000);
      GY25::write(0XA5);  // request the data
      GY25::write(0X52);
      GY25::horizontal_angle = 0;
      GY25::ggg_cache = 0;
      GY25::ggg_old = 0;
      GY25::strafe = 0; 
      GY25::strafe_timer = 0;
    }

    void calibration() {
      // setup time
      delay(3000);
      // Kalibrasi Tilt
      GY25::write(0xA5);
      GY25::write(0x54);
      // Jeda sebelum kalibrasi heading
      delay(1000);
      // Kalibrasi Heading
      GY25::write(0xA5);
      GY25::write(0x55);
      delay(100);
    }

    void update() {
      while (GY25::available()) {
        GY25::Re_buf[GY25::counter] = (unsigned char)GY25::read();
        if (GY25::counter == 0 && GY25::Re_buf[0] != 0xAA) return;
        GY25::counter++;
        if (GY25::counter == 8) {
          GY25::counter = 0;
          GY25::sign = 1;
        }
        // Serial.println("!");
      }
      if (GY25::sign) {
        GY25::sign = 0;
        if (GY25::Re_buf[0] == 0xAA && GY25::Re_buf[7] == 0x55) {
          GY25::angle[0] = (GY25::Re_buf[1] << 8 | GY25::Re_buf[2]) / 100;
          GY25::angle[1] = (GY25::Re_buf[3] << 8 | GY25::Re_buf[4]) / 100;
          GY25::angle[2] = (GY25::Re_buf[5] << 8 | GY25::Re_buf[6]) / 100;
          #ifdef ESP32
          for (int i = 0; i<3; i++) {
            if (GY25::angle[i]>180) {
              GY25::angle[i] -= 654;
            } 
          }
          #endif
          // GY25::print();
          // gy25.update();
          int a = GY25::angle[0];
          if (a>100 && ggg_old<-100) GY25::ggg_cache -= 360;
          if (a<-100 && ggg_old>100) GY25::ggg_cache += 360;
          GY25::ggg_old = a;
          GY25::horizontal_angle = a + GY25::ggg_cache;
        }
      }
    }

    void print() {
      Serial.print(GY25::angle[0]);
      Serial.print(" ");
      Serial.print(GY25::angle[1]);
      Serial.print(" ");
      Serial.print(GY25::angle[2]);
      Serial.print(" ");
      Serial.print(GY25::horizontal_angle);
      Serial.println();
    }

    // очередной костыль, который появился в рамках подготовки к кубку РТК высшая лига, IRS-1
    long int getHorizonlalAngle() {
      GY25::update();
	    if (GY25::strafe_timer<millis()) {
        GY25::strafe += GY25_STRAFE_ANDLE;
        GY25::strafe_timer = millis() + GY25_STRAFE_DT;
      }
	    return GY25::horizontal_angle+GY25::strafe;
    }
    void delayUpdate(long int t) {
      for (t += millis(); t>millis();) {
        GY25::getHorizonlalAngle();
      }
    }
    void setupStrafe() {
      // возможно эта фича вообще нафиг не нужна, но вроде что-то такое было
      GY25::strafe_timer = millis() + GY25_STRAFE_DT;
    }
  private:
    #ifndef GY25_SERIAL
    using SoftwareSerial::SoftwareSerial;
    #endif
    unsigned char counter = 0;  //buffer where the received data will be stored
    unsigned char sign = 0;
    unsigned char Re_buf[8];
    long int ggg_cache,ggg_old;
    long int strafe, strafe_timer;

    #ifdef GY25_SERIAL
    void begin(int bod) {
      GY25_SERIAL.begin(bod);
    }
    void write(char a) {
      GY25_SERIAL.write(a);
    }
    int available() {
      // Serial.println(GY25_SERIAL.available());
      return GY25_SERIAL.available();
    }
    unsigned char read() {
      return GY25_SERIAL.read();
    }
    #endif
};