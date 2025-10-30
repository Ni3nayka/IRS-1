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
  // Motors.runs(30, 30);
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

void serialDataParser() {
  // ввод управяющих данных из монитора порта
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    // Разделяем команду на части
    int firstSpace = input.indexOf(' ');
    if (firstSpace != -1) {
      String command = input.substring(0, firstSpace);
      String args = input.substring(firstSpace + 1);
      args.trim();
      
      // Обработка команды для моторов
      if (command == "m") {
        int secondSpace = args.indexOf(' ');
        if (secondSpace != -1) {
          int motorNum = args.substring(0, secondSpace).toInt();
          int speed = args.substring(secondSpace + 1).toInt();
          Motors.run(motorNum, speed);
          Serial.print("Мотор ");
          Serial.print(motorNum);
          Serial.print(" запущен со скоростью ");
          Serial.println(speed);
        }
      }
      // Обработка команды для сервоприводов
      else if (command == "s") {
        int secondSpace = args.indexOf(' ');
        if (secondSpace != -1) {
          int servoNum = args.substring(0, secondSpace).toInt() - 1;
          int angle = args.substring(secondSpace + 1).toInt();
          if (servoNum >= 0 && servoNum < servoCount) {
            smoothMoveServo(servoNum,angle);
            // servos[servoNum].write(angle);
            Serial.print("Серва ");
            Serial.print(servoNum + 1);
            Serial.print(": Угол = ");
            Serial.println(angle);
          } else {
            Serial.print("Ошибка: Недопустимый номер сервы (1-");
            Serial.print(servoCount);
            Serial.println(")");
          }
        }
      }
      else if (command == "e") { // опросить энкодеры - Тут короче лютейший говнокод, ибо время
        int secondSpace = args.indexOf(' ');
        if (secondSpace != -1) {
          int arg1 = args.substring(0, secondSpace).toInt();
          int arg2 = args.substring(secondSpace + 1).toInt();
          if (arg1 == 0 && arg2 == 0) {
            resetEncoders();
            Serial.println("Энкодеры обнулены");
          } else {
            printEncoders();
          }
        }
      }
      else if (command == "E") { // запустить робота по энкодерам - Тут короче лютейший говнокод, ибо время
        // метода - E f r
        // где f - вперед (или назад, если значение отрицательное) - в см
        // r - вправо (или влево, если значение отрицательное) - в градусах
        int secondSpace = args.indexOf(' ');
        if (secondSpace != -1) {
          String servoNumStr = args.substring(0, secondSpace);
          String angleStr = args.substring(secondSpace + 1);
          
          int forward = servoNumStr.toInt();  // Нумерация с 1
          int right = angleStr.toInt();

          Serial.println("Едем по энкодерам");
          // Serial.println(forward);
          // Serial.println(right);
          if (forward!=0) runGyro(forward);
          else turnGyro(right);
          Serial.println("Доехали по энкодерам &"); // & - символ, чтобы детектить его в выводе на компе
        }
      }
      else {
        Serial.println("Ошибка: Неизвестная команда. Используйте 'm' или 's' или 'e' или 'E'");
      }
    }
    else {
      Serial.println("Ошибка: Неверный формат команды");
    }
  }
}
