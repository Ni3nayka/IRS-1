#pragma once

#include "Adafruit_TCS34725.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);

#include <Servo.h>
Servo myservo; 

int global_state_hight_arm = 1;

void disable(unsigned long int t=0) {
  if (t<millis()) {
    myservo.detach();
    motors.runs(0,0,0,-20); delay(3000);
    motors.runs();
    while(1);
  }
  
}

void writeHight(int state, int real_state = -1) {
  unsigned long int t = millis() + 2000; // timeout
  if (real_state!=-1) global_state_hight_arm = real_state;
  int POROG = 500;
  int PIN = A0;
  if (state==global_state_hight_arm) return;
  bool down_move = state<global_state_hight_arm;

  // motors.run(3,10); // 10 - вниз, -10 - вверх
  // motors.run(3,20); // вниз
  // motors.run(3,-30); // вверх

  if (down_move) motors.run(4,-20); // вниз
  else motors.run(4,60); // вверх

  while (analogRead(PIN)<POROG) disable(t); // пока черный
  delay(50);
  while (analogRead(PIN)>POROG) disable(t); // пока белый
  if (down_move) delay(200);
  else delay(90);

  if (down_move) motors.run(4,50);
  else motors.run(4,-50);
  delay(50);
  motors.run(4,0);

  global_state_hight_arm = global_state_hight_arm + (int(!down_move)*2-1);
  //if (global_state_hight_arm!=state) 
  writeHight(state);
}

#define COLOR_WHITE 1
#define COLOR_BLACK 2
#define COLOR_YELLOW 3
#define COLOR_RED 4
#define COLOR_GREEN 5
#define COLOR_BLUE 6

int getColor() { // 
  int r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);
  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);

  if (0) {
    Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
    Serial.println(" ");
  }


  // color:
  // black:  Color Temp: 3211 K - Lux: 107 -  R: 296  G: 195  B: 143  C: 628  
  // yellow: Color Temp: 2331 K - Lux: 1632 - R: 3281 G: 2085 B: 810  C: 6334  
  // white:  Color Temp: 3426 K - Lux: 1594 - R: 3307 G: 2510 B: 1767 C: 7762  
  // blue:   Color Temp: 6576 K - Lux: 374 -  R: 529  G: 680  B: 720  C: 1935  
  // green:  Color Temp: 5083 K - Lux: 869 -  R: 614  G: 953  B: 595  C: 2203  
  // red:    Color Temp: 1948 K - Lux: -286 - R: 2044 G: 413  B: 376  C: 2653  

  if      (r>1500 && g>1500 && b>1500) return COLOR_WHITE;
  else if (r<500 && g<500 && b<500) return COLOR_BLACK;
  else if (r>1500 && g>1500 && b<1500) return COLOR_YELLOW;
  else if (r>1500 && g<1500 && b<1500) return COLOR_RED;
  if (g>800) return COLOR_GREEN;
  else return COLOR_BLUE;
}

int readUltrasonarBasic(int pin = 11) {
  int duration, cm;

  pinMode(pin, OUTPUT);

  // Сначала генерируем короткий импульс длительностью 2-5 микросекунд.

  digitalWrite(pin, LOW);
  delayMicroseconds(2); // 5
  digitalWrite(pin, HIGH);

  // Выставив высокий уровень сигнала, ждем около 10 микросекунд. В этот момент датчик будет посылать сигналы с частотой 40 КГц.
  delayMicroseconds(10);
  digitalWrite(pin, LOW);

  pinMode(pin, INPUT);
  // delayMicroseconds(10);

  //  Время задержки акустического сигнала на эхолокаторе.
  duration = pulseIn(pin, HIGH);

  // Теперь осталось преобразовать время в расстояние
  cm = (duration / 2) / 29.1;

  // Serial.print("Расстояние до объекта: ");
  // Serial.print(cm);
  // Serial.println(" см.");

  // // Задержка между измерениями для корректной работы скеча
  // delay(250);
  cm = constrain(cm,0,100);
  if (cm<3) cm = 100;
  return cm;
}

int readUltrasonar(int pin = 11) {
  //return (readUltrasonarBasic(pin)+readUltrasonarBasic(pin)+readUltrasonarBasic(pin))/3;
  return (readUltrasonarBasic(pin)+readUltrasonarBasic(pin))/2;
  // return readUltrasonarBasic(pin);
}