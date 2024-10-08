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
  myservo.write(90); // 55 - захват, 100 - отпустить
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
  readUltrasonar();
  if (readUltrasonar()<20) disable();
  Serial.println("OK");
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
  // line();
  // turnLeft();
  // findObject();
  // line(1);
  // runEncLeft(360);
  // line();
  
  // line(1);
  // runEncLeft(90);
  // findObject();
  // line(1);
  // line(1);
  // line(1);
  // runEncForward(40);
  // motors.run(4,-20); delay(1500); motors.runs();
  // myservo.write(90);
  // runEncForward(-40);
  // runEncLeft(180);

  // runEncLeft(360);

  // line(1);
  //while(1) {
    // line(1);
    // int a = 0;
    // a = findObject2();
    // if (a!=0) {
      
    //   //if (a==3) break;
    //   line(1);
    //   line(1);
    // }
    // else {
    //   line(1);
    //   runEncLeft(-90);
    //   line(1);
    //   runEncLeft(-90);
    //   line(1);
    //   //break;
    // }
  //}
  // line(2);
  // findObject2();
  // myservo.write(90);

  line();
  int a = 0;
  a = findObject2();
  if (a!=0) otvozBanki();

  // writeHight(3);
}

int global_x = 0, global_n = 0;

void otvozBanki() { //int color
  line(3);
  runEncLeft(90);
  line(2);
  runEncLeft(-90);
  writeHight(2); ///////////////////////////////////////////////////////////////////////
  runEncForward(35);
  //motors.run(4,-20); delay(1500); motors.runs(); delay(500);
  for (int i = 65; i<90; i++) {
    myservo.write(i);
    delay(5);
  }
  delay(500);
  // runEncForward(-35);
  motors.runs(-40,-40);
  delay(800);
  motors.runs(40,40);
  delay(50);
  motors.runs();
  runEncLeft(180);
  writeHight(1);
  line();
  runEncLeft(90); 
  line(2); //////////////////////////////////////////////////// 1-3
  runEncLeft(-90);
  line(3);
}

void loop() {
  // getPIDError();

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

int global_color = 0;

int findObject2() {
  delay(500);
  int a = 999;
  for (int i = 0; i< 10; i++) a = min(a,readUltrasonar());
  if (a<25) {
    runEncForward(15,40);
    myservo.write(65);
    delay(500);
    global_color = myColor();
    Serial.println(global_color);
    motors.run(4,60); delay(300); motors.runs();
    runEncForward(-15,40);
    runEncLeft(180);
    return 1;
  }
  else {
    runEncLeft(30);
    a = 999; for (int i = 0; i< 10; i++) a = min(a,readUltrasonar());
    if (a<25) {
      runEncForward(15,40);
      myservo.write(65);
      delay(500);
      global_color = myColor();
      Serial.println(global_color);
      motors.run(4,60); delay(300); motors.runs();
      runEncForward(-15,40);
      runEncLeft(150);
      return 2;
    }
    else {
      runEncLeft(-60);
      a = 999; for (int i = 0; i< 10; i++) a = min(a,readUltrasonar());
      if (a<25) {
        runEncForward(15,40);
        myservo.write(65);
        delay(500);
        global_color = myColor();
        Serial.println(global_color);
        motors.run(4,60); delay(300); motors.runs();
        runEncForward(-15,40);
        runEncLeft(-150);
        return 3;
      }
    } 
  }
  runEncLeft(-150);
  return 0;
}

bool findObject() {
  enc1.clear();
  enc2.clear();
  motors.runs(32,-32);
  int a = 100;
  unsigned long int t = millis()+700;
  bool end = 0;
  while (a>20 && !end) {
    a = readUltrasonar();
    if (t<millis() && bum.getLineAnalog(5)<POROG_BLACK_LINE) end = 1;
  }
  long int angle = (abs(enc1.get())+abs(enc2.get()))/(TRANSLATE_ANGLE_TO_ENC_PARROT*2);
  if (!end) {
    a += 5;
    motors.runs(-50,50);
    delay(100);
    runEncForward(a+5,40);
    myservo.write(65);
    delay(500);
    getColor();
    runEncForward(-a-12,40);
    // writeHight(3);// delay(1000);
    motors.run(4,60); delay(300); motors.runs();
    // myservo.write(90);
    // delay(1000);
    // writeHight(1);// delay(1000);
    // доворот
    // motors.runs(40,-40);
    // while (bum.getLineAnalog(5)>POROG_BLACK_LINE);
    // motors.runs(-50,50);
    // delay(100);
    // motors.runs();
    // return 1;
  }
  // доворот + разворот
  runEncLeft(angle+100);
  return 0;
}