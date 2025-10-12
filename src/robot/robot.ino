/*
   code write for project:
   https://github.com/Ni3nayka/IRS-1/

   author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
   write:  October 2024
   modify: September 2025
*/

#include <Robot_L298P.h>  // Библиотека для моторов
#include "myServo.h"      // Библиотека для сервоприводов

// Массив пинов сервоприводов
uint8_t servoPins[] = {2, 9};
const uint8_t servoCount = sizeof(servoPins) / sizeof(servoPins[0]);

#define ENC_POROG 50
#define ENC_TIME 500

#define ENC_FORWARD_KP 1
#define ENC_FORWARD_KD 2
#define ENC_FORWARD_ALIGNMENT_KP 5 // выравнивание колес друг относительно друга 
#define ENC_FORWARD_ALIGNMENT_KI 0 // выравнивание колес друг относительно друга 
#define ENC_TURN_KP 5.0
#define ENC_TURN_KD 4

#define ENC_ANGLE_TO_PARROT 8.88
#define ENC_CM_TO_PARROT 64.5

#define ENC_MOTOR_MAX_SPEED 70
#define ENC_MOTOR_MAX_SPEED_SLOW 30
#define ENC_MOTOR_R_BOOST 0.9

void runEnc(long int forward = 0, long int right = 0, int max_speed=ENC_MOTOR_MAX_SPEED) {
  if (forward!=0) right = 0;
  long int enc_a_target = Robot.enc_A+forward*ENC_CM_TO_PARROT+right*ENC_ANGLE_TO_PARROT;
  long int enc_b_target = Robot.enc_B+forward*ENC_CM_TO_PARROT-right*ENC_ANGLE_TO_PARROT;
  //slow start
  if (forward!=0) {
    for (int i = 0; i<ENC_MOTOR_MAX_SPEED; i++) {
      long int e_d = (Robot.enc_A-enc_a_target)-(Robot.enc_B-enc_b_target); ///////////////////////////////////////// ДОДЕЛАТЬ
      e_d*=ENC_FORWARD_ALIGNMENT_KP;
      // моторы (энкодеры перепутаны местами)
      long int m_a = constrain(i-e_d, -ENC_MOTOR_MAX_SPEED,ENC_MOTOR_MAX_SPEED);
      long int m_b = constrain(i+e_d, -ENC_MOTOR_MAX_SPEED,ENC_MOTOR_MAX_SPEED)*ENC_MOTOR_R_BOOST;
      Robot.motors(m_a, m_b);
      delay(4);
    }
  }
  long int time = millis()+ENC_TIME;
  long int e_a_old = 0, e_b_old = 0;
  long int i_d = 0;
  while (time>millis()) {
    if ((abs(Robot.enc_A-enc_a_target)>ENC_POROG) || abs(Robot.enc_B-enc_b_target)>ENC_POROG) {
      time = millis()+ENC_TIME;
    }
    // PID для движения тупо вперед
    // A
    long int e_a = enc_a_target-Robot.enc_A;
    long int p_a = e_a;
    long int d_a = e_a - e_a_old;
    e_a_old = e_a;
    if (forward!=0) {
      p_a*=ENC_FORWARD_KP;
      d_a*=ENC_FORWARD_KD;
    }
    else {
      p_a*=ENC_TURN_KP;
      d_a*=ENC_TURN_KD;
    }
    // B
    long int e_b = enc_b_target-Robot.enc_B;
    long int p_b = e_b*ENC_FORWARD_KP;
    long int d_b = (e_b - e_b_old);
    e_b_old = e_b;
    if (forward!=0) {
      p_b*=ENC_FORWARD_KP;
      d_b*=ENC_FORWARD_KD;
    }
    else {
      p_b*=ENC_TURN_KP;
      d_b*=ENC_TURN_KD;
    }
    // PID чтобы двигаться прямо
    long int e_d = (Robot.enc_A-enc_a_target)-(Robot.enc_B-enc_b_target); ///////////////////////////////////////// ДОДЕЛАТЬ
    i_d = e_d + i_d*0.96;
    long int p_d = e_d*ENC_FORWARD_ALIGNMENT_KP + i_d*ENC_FORWARD_ALIGNMENT_KI;
    if (forward!=0) {
      // ..
    }
    else {
      p_d = 0;
    }
    // p_d = 0;
    // моторы (энкодеры перепутаны местами)
    long int m_a = constrain(p_a+d_a -p_d, -ENC_MOTOR_MAX_SPEED,ENC_MOTOR_MAX_SPEED);
    long int m_b = constrain(p_b+d_b +p_d, -ENC_MOTOR_MAX_SPEED,ENC_MOTOR_MAX_SPEED)*ENC_MOTOR_R_BOOST;
    Robot.motors(m_a, m_b);
    // Serial.print(e_a);
    // Serial.print(" ");
    // Serial.println(e_b);
  }
  Robot.motors(0, 0);
}

void setup() {
  Serial.begin(9600);
  Robot.setup();  // Инициализация моторов
  ServoController.setupServo(servoPins, servoCount); // Инициализация сервоприводов через нашу библиотеку
  // Устанавливаем стартовое положение - 90 градусов
  for (int i = 0; i < servoCount; i++) {
    ServoController.servoWrite(i, 90);
  }
  Serial.println("Система готова. Форматы команд:");
  Serial.println("Моторы: m ЛЕВЫЙ_МОТОР ПРАВЫЙ_МОТОР");
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

  delay(3000);

  for (int i = 0; i<8; i++) {
    runEnc(80);
    // delay(1000);
    runEnc(0,90);
    // delay(1000);
  }

  // runEnc(100);
  // delay(1000);
  // runEnc(0,180);
  // delay(1000);
  // runEnc(100);
  // delay(1000);
  // runEnc(0,-180);
  // delay(1000);
}

void loop() {
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
          String leftStr = args.substring(0, secondSpace);
          String rightStr = args.substring(secondSpace + 1);
          
          int leftSpeed = leftStr.toInt();
          int rightSpeed = rightStr.toInt();
          
          Robot.motors(leftSpeed, rightSpeed);
          
          Serial.print("Моторы: Левый = ");
          Serial.print(leftSpeed);
          Serial.print(", Правый = ");
          Serial.println(rightSpeed);
        }
      }
      // Обработка команды для сервоприводов
      else if (command == "s") {
        int secondSpace = args.indexOf(' ');
        if (secondSpace != -1) {
          String servoNumStr = args.substring(0, secondSpace);
          String angleStr = args.substring(secondSpace + 1);
          
          int servoNum = servoNumStr.toInt() - 1;  // Нумерация с 1
          int angle = angleStr.toInt();
          
          if (servoNum >= 0 && servoNum < servoCount) {
            ServoController.servoWrite(servoNum, angle);
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
          String servoNumStr = args.substring(0, secondSpace);
          String angleStr = args.substring(secondSpace + 1);
          
          int servoNum = servoNumStr.toInt();  // Нумерация с 1
          int angle = angleStr.toInt();
          
          if (angle==0 && servoNum==0) {
            Robot.enc_A = 0;
            Robot.enc_B = 0;
            Serial.println("Энкодеры обнулены");
          } else {
            Serial.println("Показания энкодеров:");
            Serial.println(Robot.enc_A); // Энкодеры перепутаны местами
            Serial.println(Robot.enc_B);
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
  
  // Обновление состояния сервоприводов
  ServoController.servoUpdate();
}