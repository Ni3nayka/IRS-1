#pragma once

#include "Adafruit_TCS34725.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);

#include <Servo.h>
Servo myservo; 

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