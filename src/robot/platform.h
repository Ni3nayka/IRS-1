#pragma once

#include <iarduino_I2C_Bumper.h>                           //   Подключаем библиотеку для работы с бампером I2C-flash.
iarduino_I2C_Bumper bum(0x09); 

#include <arduino_encoder.h>
Encoder enc1;
Encoder enc2;

#define TRANSLATE_CM_TO_ENC_PARROT 118
#define TRANSLATE_ANGLE_TO_ENC_PARROT 17.5 // 15.5

#define MOTOR_SPEED 80

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
    long int pid = p*1.5 + d*400;
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

// black line
// #define POROG_BLACK_LINE 1300 // 2300 - white    300 - black
// #define LINE_PID_K_P 0.05
// #define MOTOR_SPEED 50
// #define NUMBER_L_IK 3
// #define NUMBER_R_IK 7

// white line
#define LINE_PID_K_P 0.8
#define LINE_PID_K_D 50
#define NUMBER_L_IK 3
#define NUMBER_R_IK 7
#define MIN_LINE 620
#define MAX_LINE 1700
#define INVERSION_LINE 
const int POROG_BLACK_LINE = (MAX_LINE+MIN_LINE)/2; // 1400 - white    380 - black

int global_line_pid_e_old = 0;


int getPIDError() {
  // Serial.println(bum.getLineAnalog(5));
  int a[7] = {0};
  for (int i = 2; i<=8; i++) {
    a[i-2] = bum.getLineAnalog(i);
    if (1) {
      #ifdef INVERSION_LINE
      a[i-2] = map(constrain(a[i-2],MIN_LINE,MAX_LINE),MIN_LINE,MAX_LINE,0,100);
      #else
      a[i-2] = map(constrain(a[i-2],MIN_LINE,MAX_LINE),MAX_LINE,MIN_LINE,0,100);
      #endif
    }
    // Serial.print(a[i-2]); Serial.print(" ");
  }
  // Serial.println();
  long int chislitel = a[0]*3+a[1]*2+a[2]-a[4]-a[5]*2-a[6]*3;
  long int znamenatel = a[0]+a[1]+a[2]+a[4]+a[5]+a[6];
  return chislitel*100/znamenatel;
}

void runLinePID() {
  // Serial.println(getPIDError());
  int e = getPIDError(); //bum.getLineAnalog(NUMBER_L_IK) - bum.getLineAnalog(NUMBER_R_IK);
  e *= -1; //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  int p = e;
  int d = e-global_line_pid_e_old;
  global_line_pid_e_old = e;
  int pid = p*LINE_PID_K_P + d*LINE_PID_K_D;
  int l = constrain(MOTOR_SPEED+pid,-10,MOTOR_SPEED);
  int r = constrain(MOTOR_SPEED-pid,-10,MOTOR_SPEED);
  // Serial.println(String(l) + " " + String(r));
  motors.runs(l,r,0,0);
}

void line(int n = 1) {
  for (int i = 0; i<n; i++) {
    for (unsigned long int t = millis()+500; t>millis();) {
      runLinePID();
    }
    while (bum.getLineAnalog(NUMBER_L_IK)<POROG_BLACK_LINE || bum.getLineAnalog(NUMBER_R_IK)<POROG_BLACK_LINE) { ////////////////////////////////////////////////////////////
      runLinePID();
    }
    for (unsigned long int t = millis()+80; t>millis();) {
      runLinePID();
    }
  }
  motors.runs(-60,-60);
  delay(100);
  motors.runs();
}

void turnLeft() {
  motors.runs(-80,60);
  while (bum.getLineAnalog(5)<POROG_BLACK_LINE);
  while (bum.getLineAnalog(5)>POROG_BLACK_LINE);
  while (bum.getLineAnalog(5)<POROG_BLACK_LINE);
  motors.runs(80,-60);
  delay(100);
  motors.runs();
}

