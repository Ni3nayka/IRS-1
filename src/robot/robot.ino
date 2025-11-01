/*
   code write for project:
   https://github.com/Ni3nayka/IRS-1/

   author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
   write:  October 2024
   modify: October 2025
*/
 
#include "robot.h"

void setup() {
  setupRobot();
	//test();
	Motors.runs(0,0,0,0);
}

void loop() {
  gy25.update();
  // serialDebugEncAndGyro();
  serialDataParser();
}

void test() {
  
  // Serial.println("start");
  turnGyro(-90);
  runGyro(22);
  turnGyro(88);//////////////////////////////////////////////
  delay(1000);
  Serial.println("start point 1 - 1/2");
  brushesOn();
  runGyro(135);
  brushesOff();
  Serial.println("end point 1 - 1/2");
  turnGyro(90);
  runGyro(50);
  turnGyro(90);
  delay(1000);
  Serial.println("start point 1 - 2/2");
  brushesOn();
  runGyro(140);
  Serial.println("end point 1 - 2/2");
  brushesOff();

  // test 1
  turnGyro(90);
  runGyro(25);
  Serial.println("end");

  // test 2
  // turnGyro(-90);
  // runGyro(25);
  // turnGyro(-90);
  // wallBack();
  // brushesOn();
  // runGyro(293);
  // brushesOff();
  // turnGyro(83);
  // brushesOn();
  // runGyro(150);
  // Motors.run(1, 70);
  // Motors.run(2, 70);
  // gy25.delayUpdate(5000);
  // Motors.run(1, 0);
  // Motors.run(2, 0);
  // gy25.delayUpdate(1000);
  // brushesOff();

  // turnGyro(90);
  
}


void wallBack() { // выравнивание по стенке
  Motors.run(1, 0);
  Motors.run(2, 0);
  gy25.delayUpdate(1000);
  Motors.run(1, -40);
  Motors.run(2, -40);
  gy25.delayUpdate(2000);
  Motors.run(1, 0);
  Motors.run(2, 0);
  gy25.delayUpdate(1000);
  // Motors.run(1, 40);
  // Motors.run(2, 40);
  // gy25.delayUpdate(500);
  // Motors.run(1, 0);
  // Motors.run(2, 0);
  // gy25.delayUpdate(1000); 
}

// ================== SERIAL ===============================================

void serialDebugEncAndGyro() { 
  // Выводим данные раз в 100 мс, не мешая вводу
  if (millis() - gy25_lastPrintTime >= 100) {
    gy25_lastPrintTime = millis();
    Serial.print("GY25 horizontal_angle: ");
    Serial.print(gy25.horizontal_angle);
    Serial.print("  ENC1: ");
    Serial.print(enc1_count);
    Serial.print("  ENC2: ");
    Serial.print(enc2_count);
    // testEnc();
    Serial.println();
  }
}



void serialDataParser() {
  // Вызываем функцию достаточно часто
  SerialData data = readSerialData();
  
  // Если получены данные, обрабатываем их
  if (data.mode != '\0') {
    // Serial.print("Режим: ");
    // Serial.println(data.mode);
    // Serial.print("Данные 1: ");
    // Serial.println(data.data_1);
    // Serial.print("Данные 2: ");
    // Serial.println(data.data_2);
    // Serial.print("Количество чисел: ");
    // Serial.println(data.data_counter);
    // Serial.println("---");
    if      (data.mode=='m' && data.data_counter==2) Motors.run(data.data_1, data.data_2);
    else if (data.mode=='M' && data.data_counter==2) Motors.runs(data.data_1, data.data_2);
    else if (data.mode=='F' && data.data_counter==1) runGyro(data.data_1);
    else if (data.mode=='R' && data.data_counter==1) turnGyro(data.data_1);
    else if (data.mode=='L' && data.data_counter==1) turnGyro(-data.data_1);
    else if (data.mode=='g' && data.data_counter==0) sendDataToSerial();
    else Serial.println("ERROR: unknow command");
  }
  
  //delay(100); // Небольшая задержка между опросами
}