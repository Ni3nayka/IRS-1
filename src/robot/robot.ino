/*
   code write for project:
   https://github.com/Ni3nayka/IRS-1/

   author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
   write:  October 2024
   modify: July 2025
*/
  // Пример использования энкодеров
  // Robot.reverse_enc_A();       // "Разворачиваем" энкодер на моторе A (если необходимо)
  // Robot.reverse_enc_B();       // "Разворачиваем" энкодер на моторе B (если необходимо)
  // Serial.println(Robot.enc_A); // Вывести значение с энкодера А
  // Serial.println(Robot.enc_B); // Вывести значение с энкодера В
  // Robot.enc_A = 0;             // Обнулим энкодер мотора А
  // Robot.enc_A = 0;             // Обнулим энкодер мотора В


#include <Robot_L298P.h>  // Библиотека для моторов
#include <Servo.h>        // Библиотека для сервоприводов

// Массив сервоприводов (пины 2 и 9)
Servo servos[2];
const int servoPins[2] = {2, 9};

void setup() {
  Serial.begin(9600);
  Robot.setup();  // Инициализация моторов
  
  // Инициализация сервоприводов
  for (int i = 0; i < 2; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90);  // Стартовое положение - 90 градусов
  }
  
  Serial.println("Система готова. Форматы команд:");
  Serial.println("Моторы: m ЛЕВЫЙ_МОТОР ПРАВЫЙ_МОТОР");
  Serial.println("Сервы: s НОМЕР_СЕРВЫ УГОЛ");
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
          
          if (servoNum >= 0 && servoNum < 2) {
            servos[servoNum].write(angle);
            Serial.print("Серва ");
            Serial.print(servoNum + 1);
            Serial.print(": Угол = ");
            Serial.println(angle);
          } else {
            Serial.println("Ошибка: Недопустимый номер сервы (1 или 2)");
          }
        }
      }
      else {
        Serial.println("Ошибка: Неизвестная команда. Используйте 'm' или 's'");
      }
    }
    else {
      Serial.println("Ошибка: Неверный формат команды");
    }
  }
}