/*
   code write for project:
   https://github.com/Ni3nayka/IRS-1/

   author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
   write:  October 2024
   modify: October 2024
*/

#include <arduino_encoder.h>
Encoder enc1;
Encoder enc2;

#include "pins.h"

#include "BTS7960_PRO.h"
BTS7960_PRO motors;

#include "i2c_tester.h"

// #include "gy-25.h"
// GY25 gy25(12,8); // (TX,RX) - пины гироскопа

#include <Wire.h>                                          // * Подключаем библиотеку для работы с аппаратной шиной I2C.
#include <iarduino_I2C_Bumper.h>                           //   Подключаем библиотеку для работы с бампером I2C-flash.
iarduino_I2C_Bumper bum(0x09); 

#include "Adafruit_TCS34725.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);

#define POROG_BLACK_LINE 1300 // 2300 - white    300 - black
#define LINE_PID_K_P 0.05
#define MOTOR_SPEED 50
#define NUMBER_L_IK 3
#define NUMBER_R_IK 7

#include <Servo.h>
Servo myservo; 

void runLinePID() {
  int e = bum.getLineAnalog(NUMBER_L_IK) - bum.getLineAnalog(NUMBER_R_IK);
  int p = e*LINE_PID_K_P;
  int l = constrain(MOTOR_SPEED+p,-10,MOTOR_SPEED);
  int r = constrain(MOTOR_SPEED-p,-10,MOTOR_SPEED);
  motors.runs(l,r,0,0);
}

void line(int n = 1) {
  for (int i = 0; i<n; i++) {
    while (bum.getLineAnalog(NUMBER_L_IK)>POROG_BLACK_LINE || bum.getLineAnalog(NUMBER_R_IK)>POROG_BLACK_LINE) {
      runLinePID();
    }
    for (unsigned long int t = millis()+60; t>millis();) {
      runLinePID();
    }
  }
  motors.runs(-50,-50);
  delay(100);
  motors.runs();
}

int global_state_hight_arm = 1;

void writeHight(int state, int real_state = -1) {
  if (real_state!=-1) global_state_hight_arm = real_state;
  int POROG = 350;
  int PIN = A0;
  if (state==global_state_hight_arm) return;
  bool down_move = state<global_state_hight_arm;

  // motors.run(3,10); // 10 - вниз, -10 - вверх
  // motors.run(3,20); // вниз
  // motors.run(3,-30); // вверх

  if (down_move) motors.run(3,20); // вниз
  else motors.run(3,-50); // вверх

  while (analogRead(PIN)<POROG); // пока черный
  delay(50);
  while (analogRead(PIN)>POROG); // пока белый
  if (down_move) delay(60);
  else delay(90);

  if (down_move) motors.run(3,-50);
  else motors.run(3,50);
  delay(50);
  motors.run(3,0);

  global_state_hight_arm = global_state_hight_arm + (int(!down_move)*2-1);
  //if (global_state_hight_arm!=state) 
  writeHight(state);
}

int getColor() { // 
  int r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);
  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);

  if (1) {
    Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
    Serial.println(" ");
  }
}

void setup() {
  // i2cTester(); ///////////////////////////////////////////////// I2C TERSER /////////////////////////////////////////////////////////////////////////
  delay(500);
  Serial.begin(9600);
  motors.setup();
  bum.begin(); 
  enc1.setup(A3,A2);
  enc2.setup(A1,A0);
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  myservo.attach(8);
  myservo.write(100); // 55 - захват, 100 - отпустить
  // gy25.setup();
  // testMotors();
  // testMotors();
  // motors.runs(100,100,0,0); delay(2000);
  // motors.runs(-50,-50,0,0); delay(2000);
  // motors.runs(50,-50,0,0); delay(2000);
  // motors.runs(-50,50,0,0); delay(2000);
  // motors.runs(0,0,20,0); delay(1000);
  motors.runs(0,0,0,0);
  //line(3);
  // runEncForward(43); // 43
  // runEncForward(-43); 
  // runEncLeft(360);
  // runEncLeft(-360);
  // writeHight(1,3); delay(1000);
  // writeHight(2); delay(1000);
  // writeHight(3); delay(1000);
}

#define TRANSLATE_CM_TO_ENC_PARROT 118
#define TRANSLATE_ANGLE_TO_ENC_PARROT 15.5

void runEncLeft(long int angle) {
  enc1.clear();
  enc2.clear();
  bool flag = 1;
  angle *= TRANSLATE_ANGLE_TO_ENC_PARROT; 
  long int e_old = 0;
  long int i = 0, i_between = 0;
  unsigned long int t = millis();
  while (flag) {
    long int e1 = angle - enc1.get();
    long int e2 = angle + enc2.get(); 
    long int e = (e1+e2)/2; 
    long int p = e;
    long int d = e-e_old;
    e_old = e;
    long int pid = p*1 + d*350;
    int l = constrain(pid,-MOTOR_SPEED,MOTOR_SPEED);
    int r = constrain(-pid,-MOTOR_SPEED,MOTOR_SPEED)*0.78;
    motors.runs(l,r);
    if (abs(e)<10) {
      if (t<millis()) flag = 0;
    }
    else t = millis() + 500;
  }
  motors.runs();
}

void runEncForward(long int cm) {
  enc1.clear();
  enc2.clear();
  bool flag = 1;
  cm *= TRANSLATE_CM_TO_ENC_PARROT;
  long int e_old = 0;
  long int i = 0, i_between = 0;
  unsigned long int t = millis();
  while (flag) {
    long int e1 = cm - enc1.get();
    long int e2 = cm - enc2.get();
    long int e = (e1+e2)/2;
    long int e_between = e1 - e2; // выравнивание между колесами
    long int p_between = e_between;
    i_between += e_between;
    long int pid_between = p_between*10 + i_between*0.01;
    long int p = e;
    long int d = e-e_old;
    e_old = e;
    i += e;
    long int pid = p*1 + i*0 + d*350;
    //pid = MOTOR_SPEED;
    int l = constrain(pid+pid_between,-MOTOR_SPEED,MOTOR_SPEED);
    int r = constrain(pid-pid_between,-MOTOR_SPEED,MOTOR_SPEED)*0.78;
    motors.runs(l,r);
    if (abs(e)<10) {
      if (t<millis()) flag = 0;
    }
    else t = millis() + 500;
  }
  motors.runs();
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

  getColor();
}