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
const uint8_t servoPins[] = {10, 11};
const uint8_t servoCount = sizeof(servoPins) / sizeof(servoPins[0]);
Servo servos[2];


// Подключаем файл с энкодерами
#include "encoders.h"
#define GY25_SERIAL Serial2
#include "gy-25.h" // Подключаем библиотеку гироскопа
// Экземпляр гироскопа (указываем RX и TX пины)
GY25 gy25; // (12, 8);
unsigned long int gy25_lastPrintTime = 0;

#define ENC_POROG 50
#define ENC_TIME 500

#define ENC_FORWARD_KP 0.5
#define ENC_FORWARD_KD 10
#define ENC_FORWARD_ALIGNMENT_KP 20 // выравнивание колес друг относительно друга 
#define ENC_TURN_KP 5.0
#define ENC_TURN_KD 4
#define ENC_GYRO_FORWARD_KP 15

#define ENC_ANGLE_TO_PARROT 17
#define ENC_CM_TO_PARROT 130

#define ENC_MOTOR_MAX_SPEED 70 // 70
#define ENC_MOTOR_R_BOOST 1

// Объявление объекта управления моторами
BTS7960_PRO Motors;

void runGyro(long int forward = 0) {
  // Адаптировано под новые энкодеры и BTS7960_PRO
  // if (forward != 0) right = 0;
  long int enc_target = enc1_count + forward * ENC_CM_TO_PARROT;
  long int time = millis() + ENC_TIME;
  long int e_old = 0;
  long int gyro_target = gy25.horizontal_angle +5; // чуть влево при старте
  while (time > millis()) {
    gy25.update();
    if ((abs(enc1_count - enc_target) > ENC_POROG)) {
      time = millis() + ENC_TIME;
    }
    // PID для движения тупо вперед
    long int e = enc_target-enc1_count;
    long int p = e;
    long int d = e - e_old;
    e_old = e;
    if (forward != 0) {
      p *= ENC_FORWARD_KP;
      d *= ENC_FORWARD_KD;
    } else {
      p *= ENC_TURN_KP;
      d *= ENC_TURN_KD;
    }

    // PID чтобы двигаться прямо
    long int e_gyro = gy25.horizontal_angle-gyro_target;
    long int p_gyro = e_gyro*ENC_GYRO_FORWARD_KP;
    // моторы
    long int m1 = constrain(p + d, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED);
    long int m2 = constrain(p + d, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED);
    m1 = constrain(m1 + p_gyro, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED) * ENC_MOTOR_R_BOOST;
    m2 = constrain(m2 - p_gyro, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED);
    Motors.run(1, m1);
    Motors.run(2, m2);
    // Serial.print(e);
    // Serial.print(" ");
    // Serial.println(e_gyro);
  }
  Motors.run(1, 0);
  Motors.run(2, 0);
  // Motors.runs(0, 0, 0, 0);
}

void turnGyro(long int right=0) {

}

void setup() {
  Serial.begin(9600);
  Motors.setup();
  gy25.setup();
  Serial2.begin(115200); // ПОТОМУ ЧТО ТУПАЯ АРДУИНА 
  for (uint8_t i = 0; i < servoCount; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90);
    gy25.update();
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
  delay(1000);
  gy25.update();

  // Motors.runs(-100, 100);
  // delay(1000);
  // Motors.runs(0, 0);

  // runEnc(0,360);
  // delay(2000);
  // runEnc(0,-360);

  runGyro(320); // 320

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
  // Motors.runs(30, 30);
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