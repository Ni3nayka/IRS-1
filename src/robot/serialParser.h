#pragma once

struct SerialData {
  char mode;
  int data_1;
  int data_2;
  int data_counter;
};

// Функция для опроса монитора порта и парсинга данных
SerialData readSerialData() {
  SerialData result;
  result.mode = '\0'; // Инициализация пустым значением
  result.data_1 = 0;
  result.data_2 = 0;
  result.data_counter = 0;
  
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Удаляем лишние пробелы
    
    if (input.length() > 0) {
      // Разбиваем строку на части
      int firstSpace = input.indexOf(' ');
      int secondSpace = input.indexOf(' ', firstSpace + 1);
      
      // Парсим букву (первый символ)
      if (input.length() >= 1) {
        result.mode = input.charAt(0);
      }
      
      // Парсим первое число
      if (firstSpace != -1) {
        String num1Str;
        if (secondSpace != -1) {
          num1Str = input.substring(firstSpace + 1, secondSpace);
        } else {
          num1Str = input.substring(firstSpace + 1);
        }
        num1Str.trim();
        if (num1Str.length() > 0) {
          result.data_1 = num1Str.toInt();
          result.data_counter++;
        }
      }
      
      // Парсим второе число
      if (secondSpace != -1) {
        String num2Str = input.substring(secondSpace + 1);
        num2Str.trim();
        if (num2Str.length() > 0) {
          result.data_2 = num2Str.toInt();
          result.data_counter++;
        }
      }
    }
  }
  
  return result;
}
