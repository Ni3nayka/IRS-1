/*
   code write for project:
   https://github.com/Ni3nayka/IRS-1/

   author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
   write:  October 2024
   modify: October 2025
*/

#include "movies.h"

void setup() {
  Serial.begin(9600);
  Motors.setup();
  gy25.setup();
  pinMode(MOSFET_PIN,OUTPUT);
  digitalWrite(MOSFET_PIN,0);
  // gy25.calibration();
  Serial2.begin(115200); // ПОТОМУ ЧТО ТУПАЯ АРДУИНА 
  // Serial1.begin(9600);
  for (uint8_t i = 0; i < servoCount; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90);
    gy25.update();
  }
  setupEncoders();
  Serial.println("Система готова. Форматы команд:");
  Serial.println("Моторы: m 1_скорость 2_скорость");
  Serial.println("Сервы: s НОМЕР_СЕРВЫ УГОЛ");
  Serial.println("Энкодеры: e 0 0 - обнулить 1 1 - запросить");
  Serial.println("Движение по энкодерам: E 0 0 - forward right");
  delay(1000);
  gy25.update();
  test();
  Motors.runs(0,0,0,0);
  digitalWrite(MOSFET_PIN,0);
}

void loop() {
  gy25.update();
  // Motors.runs(30, 30);
  // Выводим данные раз в 100 мс, не мешая вводу
  // if (millis() - gy25_lastPrintTime >= 100) {
  //   gy25_lastPrintTime = millis();
  //   Serial.print("GY25 horizontal_angle: ");
  //   Serial.print(gy25.horizontal_angle);
  //   Serial.print("  ENC1: ");
  //   Serial.print(enc1_count);
  //   Serial.print("  ENC2: ");
  //   Serial.print(enc2_count);
  //   // testEnc();
  //   Serial.println();
  // }
  serialDataParser();
}

void test() {
  // Motors.runs(-100, 100);
  // delay(1000);
  // Motors.runs(0, 0);

  // runEnc(0,360);
  // delay(2000);
  // runEnc(0,-360);

  // runGyro(320); // 320
  // turnGyro(360);


  // delay(500);
  // runGyro(200);

  // delay(1000);
  // turnGyro(180);
  // delay(1000);
  // runGyro(200);
  // delay(1000);
  // turnGyro(-180);
  // delay(1000);

  // for (int i = 0; i<16; i++) {
  //   runGyro(100);
  //   delay(1000);
  //   turnGyro(90);
  //   delay(1000);
  // }

  // smoothMoveServo(0,180);
  // Motors.run(3, 100);
  // runGyro(120);
  // Motors.run(3, 0);
  // smoothMoveServo(0,90);

  // delay(1000);
  // turnGyro(180);
  // delay(1000);
  // runGyro(100);
  // delay(1000);
  // turnGyro(-180);
  // delay(1000);

  // turnGyro(180);
  // turnGyro(-180);
  // return;



  // Serial.println("start");
  turnGyro(-90);
  runGyro(22);
  turnGyro(88);//////////////////////////////////////////////
  delay(1000);
  Serial.println("start point 1 - 1/2");
  OnMosfet();
  runGyro(135);
  OffMosfet();
  Serial.println("end point 1 - 1/2");
  turnGyro(90);
  runGyro(50);
  turnGyro(90);
  delay(1000);
  Serial.println("start point 1 - 2/2");
  OnMosfet();
  runGyro(140);
  Serial.println("end point 1 - 2/2");
  OffMosfet();

  // test 1
  turnGyro(90);
  runGyro(25);
  Serial.println("end");

  // test 2
  // turnGyro(-90);
  // runGyro(25);
  // turnGyro(-90);
  // wallBack();
  // OnMosfet();
  // runGyro(293);
  // OffMosfet();
  // turnGyro(83);
  // OnMosfet();
  // runGyro(150);
  // Motors.run(1, 70);
  // Motors.run(2, 70);
  // gyroDelay(5000);
  // Motors.run(1, 0);
  // Motors.run(2, 0);
  // gyroDelay(1000);
  // OffMosfet();

  // turnGyro(90);
  
}

void gyroDelay(long int t) {
  for (t += millis(); t>millis();) {
    updateGyroStrafe();
  }
}

void OnMosfet() {
  gyroDelay(1000);
  digitalWrite(MOSFET_PIN,1);
  gyroDelay(1000);
}
void OffMosfet() {
  gyroDelay(1000);
  digitalWrite(MOSFET_PIN,0);
  gyroDelay(1000);
}


void wallBack() { // выравнивание по стенке
  Motors.run(1, 0);
  Motors.run(2, 0);
  gyroDelay(1000);
  Motors.run(1, -40);
  Motors.run(2, -40);
  gyroDelay(2000);
  Motors.run(1, 0);
  Motors.run(2, 0);
  gyroDelay(1000);
  // Motors.run(1, 40);
  // Motors.run(2, 40);
  // gyroDelay(500);
  // Motors.run(1, 0);
  // Motors.run(2, 0);
  // gyroDelay(1000); 
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
