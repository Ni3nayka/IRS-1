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

void setup() {
  Serial.begin(9600);
  Robot.setup();  // Инициализация моторов
  
  // Robot.reverse_enc_A();       // "Разворачиваем" энкодер на моторе A (если необходимо)
  // Robot.reverse_enc_B();       // "Разворачиваем" энкодер на моторе B (если необходимо)
  // Serial.println(Robot.enc_A); // Вывести значение с энкодера А
  // Serial.println(Robot.enc_B); // Вывести значение с энкодера В
  // Robot.enc_A = 0;             // Обнулим энкодер мотора А
  // Robot.enc_A = 0;             // Обнулим энкодер мотора В

  // Инициализация сервоприводов через нашу библиотеку
  ServoController.setupServo(servoPins, servoCount);
  
  // Устанавливаем стартовое положение - 90 градусов
  for (int i = 0; i < servoCount; i++) {
    ServoController.servoWrite(i, 90);
  }
  
  Serial.println("Система готова. Форматы команд:");
  Serial.println("Моторы: m ЛЕВЫЙ_МОТОР ПРАВЫЙ_МОТОР");
  Serial.println("Сервы: s НОМЕР_СЕРВЫ УГОЛ");
  Serial.println("Энкодеры: e 0 0 - обнулить 1 1 - запросить");

  Robot.motors(20, 0);
  delay(1000);
  Robot.motors(0, 0);
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
      else if (command == "e") { // Тут короче лютейший говнокод, ибо время
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
            Serial.println(-Robot.enc_B); // Энкодеры перепутаны местами
            Serial.println(-Robot.enc_A);
          }
        }
      }
      else {
        Serial.println("Ошибка: Неизвестная команда. Используйте 'm' или 's' или 'e'");
      }
    }
    else {
      Serial.println("Ошибка: Неверный формат команды");
    }
  }
  
  // Обновление состояния сервоприводов
  ServoController.servoUpdate();
}