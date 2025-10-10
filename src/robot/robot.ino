/*
   code write for project:
   https://github.com/Ni3nayka/IRS-1/

   author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
   write:  October 2024
   modify: October 2025
*/


#include "BTS7960_PRO.h"  // Управление моторами
#include <Servo.h>         // Стандартная библиотека для сервоприводов


// Массив пинов сервоприводов
const uint8_t servoPins[] = {2, 9};
const uint8_t servoCount = sizeof(servoPins) / sizeof(servoPins[0]);
Servo servos[2];


// Подключаем файл с энкодерами
#include "encoders.h"
#include "gy-25.h" // Подключаем библиотеку гироскопа
// Экземпляр гироскопа (указываем RX и TX пины)
GY25 gy25(12, 8);
unsigned long int gy25_lastPrintTime = 0;

#define ENC_POROG 50
#define ENC_TIME 500

#define ENC_FORWARD_KP 0.5
#define ENC_FORWARD_KD 10
#define ENC_FORWARD_ALIGNMENT_KP 20 // выравнивание колес друг относительно друга 
#define ENC_TURN_KP 5.0
#define ENC_TURN_KD 4

#define ENC_ANGLE_TO_PARROT 17
#define ENC_CM_TO_PARROT 125

#define ENC_MOTOR_MAX_SPEED 70
#define ENC_MOTOR_R_BOOST 1

// Объявление объекта управления моторами
BTS7960_PRO Motors;

void runEnc(long int forward = 0, long int right = 0) {
  // Адаптировано под новые энкодеры и BTS7960_PRO
  if (forward != 0) right = 0;
  long int enc1_target = enc1_count - forward * ENC_CM_TO_PARROT + right * ENC_ANGLE_TO_PARROT;
  long int enc2_target = enc2_count - forward * ENC_CM_TO_PARROT - right * ENC_ANGLE_TO_PARROT;
  long int time = millis() + ENC_TIME;
  long int e1_old = 0, e2_old = 0;
  while (time > millis()) {
    if ((abs(enc1_count - enc1_target) > ENC_POROG) || abs(enc2_count - enc2_target) > ENC_POROG) {
      time = millis() + ENC_TIME;
    }
    // PID для движения тупо вперед
    // A
    long int e1 = enc1_count - enc1_target;
    long int p1 = e1;
    long int d1 = e1 - e1_old;
    e1_old = e1;
    if (forward != 0) {
      p1 *= ENC_FORWARD_KP;
      d1 *= ENC_FORWARD_KD;
    } else {
      p1 *= ENC_TURN_KP;
      d1 *= ENC_TURN_KD;
    }
    // B
    long int e2 = enc2_count - enc2_target;
    long int p2 = e2 * ENC_FORWARD_KP;
    long int d2 = (e2 - e2_old);
    e2_old = e2;
    if (forward != 0) {
      p2 *= ENC_FORWARD_KP;
      d2 *= ENC_FORWARD_KD;
    } else {
      p2 *= ENC_TURN_KP;
      d2 *= ENC_TURN_KD;
    }
    // PID чтобы двигаться прямо
    long int e_d = (enc1_count - enc1_target) - (enc2_count - enc2_target); // ДОДЕЛАТЬ
    long int p_d = e_d * ENC_FORWARD_ALIGNMENT_KP;
    if (forward != 0) {
      // ..
    } else {
      p_d = 0;
    }
    // p_d = 0;
    // моторы
    long int m1 = constrain(p1 + d1 + p_d, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED) * ENC_MOTOR_R_BOOST;
    long int m2 = constrain(p2 + d2 - p_d, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED);
    Motors.runs(m1, m2, 0, 0); // только два мотора
    // Serial.print(e1);
    // Serial.print(" ");
    // Serial.println(e2);
  }
  Motors.runs(0, 0, 0, 0);
}


void setup() {
  Serial.begin(9600);
  Motors.setup();
  gy25.setup();
  for (uint8_t i = 0; i < servoCount; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90);
  }
  pinMode(ENC1_A, INPUT_PULLUP);
  pinMode(ENC1_B, INPUT_PULLUP);
  pinMode(ENC2_A, INPUT_PULLUP);
  pinMode(ENC2_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC1_A), enc1A_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC1_B), enc1B_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC2_A), enc2A_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC2_B), enc2B_ISR, CHANGE);
  Serial.println("Система готова. Форматы команд:");
  Serial.println("Моторы: m 1_скорость 2_скорость");
  Serial.println("Сервы: s НОМЕР_СЕРВЫ УГОЛ");
  Serial.println("Энкодеры: e 0 0 - обнулить 1 1 - запросить");
  Serial.println("Движение по энкодерам: E 0 0 - forward right");
  
  // Robot.motors(20, 0);
  // delay(1000);
  // Robot.motors(0, 0);

  // runEnc(0,360);
  // delay(2000);
  // runEnc(0,-360);

  // runEnc(50);

  // runEnc(300);
  // delay(1000);
  // runEnc(0,180);
  // delay(1000);
  // runEnc(300);
  // delay(1000);
  // runEnc(0,-180);
  // delay(1000);
}

void loop() {
  gy25.update();
  // Выводим данные раз в 100 мс, не мешая вводу
  if (millis() - gy25_lastPrintTime >= 100) {
    gy25_lastPrintTime = millis();
    Serial.print("GY25 horizontal_angle: ");
    Serial.print(gy25.horizontal_angle);
    Serial.print(" | ENC1: ");
    Serial.print(enc1_count);
    Serial.print(" ENC2: ");
    Serial.println(enc2_count);
  }
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
            servos[servoNum].write(angle);
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
          runEnc(forward, right);
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