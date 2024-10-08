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

  myservo.attach(8);
  myservo.write(100); // 55 - захват, 100 - отпустить
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
    disable();
    while (1);
  }
  motors.runs(0,0,0,0);
  // Serial.println(readUltrasonar());
  if (readUltrasonar()<20) disable();
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // gy25.setup();
  // testMotors();
  // testMotors();
  // motors.runs(100,100,0,0); delay(2000); motors.runs();
  // motors.runs(-50,-50,0,0); delay(2000);
  // motors.runs(50,-50,0,0); delay(2000);
  // motors.runs(-50,50,0,0); delay(2000);
  // motors.runs(0,0,0,-20); delay(3000);
  // motors.runs(0,0,0,0);
  // runEncForward(43); // 43
  // runEncForward(-43); 
  // runEncLeft(360);
  // runEncLeft(-360);
  // writeHight(1,3); delay(1000);
  // writeHight(2); delay(1000);
  // writeHight(3); delay(1000);
  // writeHight(1,3); delay(1000);
  // writeHight(1); delay(1000);
  // writeHight(1); delay(1000);
  // motors.runs(0,0,0,-20); delay(1000); motors.runs();
  // line(4);
  // runEncLeft(180);
  // line(4);
  // turnLeft();
  findObject();
  // line(1);
  // runEncLeft(360);
  // line();
  
}


void loop() {

  // line();
  // runEncLeft(-180);
  // delay(1000);
  // line();
  // runEncLeft(180);
  // delay(1000);

  // line(3);
  // runEncLeft(-90);
  // line(2);
  // runEncLeft(-90);
  // line(2);
  // runEncLeft(-90);
  // line(2);
  // runEncLeft(90);
  // line(1);
  // runEncLeft(180);
  // delay(1000);

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
  // Serial.println(getPIDError());
  // getPIDError();
  // runLinePID();
  // Serial.println(readUltrasonar());
  // Serial.println(analogRead(A0));
  // findObject();
}

bool findObject() {
  enc1.clear();
  enc2.clear();
  motors.runs(35,-35);
  int a = 100;
  unsigned long int t = millis()+700;
  bool end = 0;
  while (a>30 && !end) {
    a = readUltrasonar();
    if (t<millis() && bum.getLineAnalog(5)<POROG_BLACK_LINE) end = 1;
  }
  long int angle = (abs(enc1.get())+abs(enc2.get()))/(TRANSLATE_ANGLE_TO_ENC_PARROT*2);
  if (!end) {
    a += 5;
    motors.runs(-50,50);
    delay(100);
    runEncForward(a,40);
    myservo.write(55);
    delay(500);
    getColor();
    runEncForward(-a+2,40);
    writeHight(3);// delay(1000);
    myservo.write(100);
    // delay(1000);
    writeHight(1);// delay(1000);
    // доворот
    // motors.runs(40,-40);
    // while (bum.getLineAnalog(5)>POROG_BLACK_LINE);
    // motors.runs(-50,50);
    // delay(100);
    // motors.runs();
    // return 1;
  }
  // доворот + разворот
  runEncLeft(-250+angle);
  return 0;
}