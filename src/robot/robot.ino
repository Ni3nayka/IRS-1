/*
   code write for project:
   https://github.com/Ni3nayka/IRS-1/

   author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
   write:  October 2024
   modify: October 2024
*/

#include <Wire.h> 
#include "BTS7960_PRO.h"
BTS7960_PRO motors;

// #include "pins.h"
#include "platform.h"
#include "arm.h"
#include "i2c_tester.h"

void setup() {
  // i2cTester(); ///////////////////////////////////////////////// I2C TERSER /////////////////////////////////////////////////////////////////////////
  delay(500);
  Serial.begin(9600);
  motors.setup();
  bum.begin(); 
  enc1.setup(A3,A2);
  enc2.setup(A1,12);
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  myservo.attach(8);
  myservo.write(100); // 55 - захват, 100 - отпустить
  motors.runs(0,0,0,0);
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // gy25.setup();
  // testMotors();
  // testMotors();
  // motors.runs(100,100,0,0); delay(2000); motors.runs();
  // motors.runs(-50,-50,0,0); delay(2000);
  // motors.runs(50,-50,0,0); delay(2000);
  // motors.runs(-50,50,0,0); delay(2000);
  // motors.runs(0,0,20,0); delay(1000);
  // motors.runs(0,0,0,0);
  // runEncForward(43); // 43
  // runEncForward(-43); 
  // runEncLeft(360);
  // runEncLeft(-360);
  // writeHight(1,3); delay(1000);
  // writeHight(2); delay(1000);
  // writeHight(3); delay(1000);
  // line(4);
  // runEncLeft(180);
  // line(4);
  // turnLeft();
}

void loop() {
  // gy25.update();
  // if (t<millis()) {
  //   gy25.print();
  //   t = millis() + 100;
  // long int e = rotation - gy25.horizontal_angle;
  // }
  
  // Serial.println( bum.getLineAnalog(5) ); 
  // Serial.println(enc1.get()); // Выводим показания энкодера 1
  // Serial.println(enc2.get());
  // delay(1000);
  // getColor();
  // runLinePID();
  Serial.println(readUltrasonar());
}