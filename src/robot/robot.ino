/*
   code write for project:
   https://github.com/Ni3nayka/IRS-1/

   author: Egor Bakay <egor_bakay@inbox.ru> Ni3nayka
   write:  October 2024
   modify: September 2025
*/

#include <Robot_L298P.h>  // Библиотека для моторов
#include "myServo.h"      // Библиотека для сервоприводов

#include "serialParser.h"
long int gy25_lastPrintTime = 0;

#define MOTOR_MOSFET_PIN A0
// #define MOTOR_MOSFET_SPEED 200

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

/*
24309 24054
27061 26893
28504 28342
*/ 
#define ENC_BETWEEN_ERROR_RIGHT_K 1.0075

#define ENC_ANGLE_TO_PARROT 12 //8.88
#define ENC_CM_TO_PARROT 64.5

#define ENC_MOTOR_MAX_SPEED 70
#define ENC_MOTOR_MAX_SPEED_SLOW 40
#define SLOW ENC_MOTOR_MAX_SPEED_SLOW
#define ENC_MOTOR_R_BOOST 0.92 //0.9

unsigned long int debug_time = 0;

void runEnc(long int forward = 0, long int right_angle = 0, int max_speed=ENC_MOTOR_MAX_SPEED) {
  if (forward!=0) right_angle = 0;
  long int enc_a_target = Robot.enc_A+forward*ENC_CM_TO_PARROT+right_angle*ENC_ANGLE_TO_PARROT;
  long int enc_b_target = Robot.enc_B+forward*ENC_CM_TO_PARROT-right_angle*ENC_ANGLE_TO_PARROT;
  enc_b_target /= ENC_BETWEEN_ERROR_RIGHT_K;
  //slow start
  if (forward!=0) {
    for (int i = 0; i<max_speed; i++) {
      long int e_d = 0;//(Robot.enc_A-enc_a_target)-(Robot.enc_B-enc_b_target)*ENC_BETWEEN_ERROR_RIGHT_K; ///////////////////////////////////////// ДОДЕЛАТЬ
      e_d*=ENC_FORWARD_ALIGNMENT_KP;
      // моторы (энкодеры перепутаны местами)
      long int m_a = constrain(i-e_d, -max_speed,max_speed);
      long int m_b = constrain(i+e_d, -max_speed,max_speed)*ENC_MOTOR_R_BOOST;
      Robot.motors(m_a, m_b);
      delay(4);
    }
  }
  long int time = millis()+ENC_TIME;
  long int e_a_old = 0, e_b_old = 0;
  long int i_d = 0;
  while (time>millis()) {
    // serial
		SerialData data = readSerialData();
  	if (data.mode != '\0') {
			if      (data.mode=='S' && data.data_counter==0) break;
			//else if (data.mode=='e' && data.data_counter==1) serial_e_for_gyro = data.data_1;
			else if (data.mode=='g' && data.data_counter==0) sendDataToSerial();
			else Serial.println("ERROR: unknow command");
		}
    // dfgthyjkl
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
    long int e_b = (enc_b_target-Robot.enc_B)*ENC_BETWEEN_ERROR_RIGHT_K;
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
    long int e_d = 0;//(Robot.enc_A-enc_a_target)-(Robot.enc_B-enc_b_target)*ENC_BETWEEN_ERROR_RIGHT_K; ///////////////////////////////////////// ДОДЕЛАТЬ
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
    long int m_a = constrain(p_a+d_a -p_d, -max_speed,max_speed);
    long int m_b = constrain(p_b+d_b +p_d, -max_speed,max_speed)*ENC_MOTOR_R_BOOST;
    Robot.motors(m_a, m_b);
    // Serial.print(e_a);
    // Serial.print(" ");
    // Serial.println(e_b);
  }
  Robot.motors(0, 0);
}

void forward(long int forward = 0, int max_speed=ENC_MOTOR_MAX_SPEED) {
  runEnc(forward,0,max_speed);
}

void right(long int right_angle = 0) {
  runEnc(0,right_angle);
}

void left(long int right_angle = 0) {
  runEnc(0,-right_angle);
}

void setup() {
  Serial.begin(9600);
  Robot.setup();  // Инициализация моторов
  ServoController.setupServo(servoPins, servoCount); // Инициализация сервоприводов через нашу библиотеку
  // Устанавливаем стартовое положение - 90 градусов
  for (int i = 0; i < servoCount; i++) {
    ServoController.servoWrite(i, 90);
  }
  // delay(3000);
  // test();
  Robot.motors(0, 0);
}

void loop() {
  serialDataParser();
  // serialDebugEncAndGyro();
  // Обновление состояния сервоприводов
  ServoController.servoUpdate();
}


void test() {
  //runEnc(0,90);
  digitalWrite(MOTOR_MOSFET_PIN, 1);
  delay(1000);
  Robot.motors(80, 100);
  delay(3000);
  // forward(100,SLOW); 
  // right(90);
  // right(720);
}


// ================== SERIAL ===============================================

void serialDebugEncAndGyro() { 
  // Выводим данные раз в 100 мс, не мешая вводу
  if (millis() - gy25_lastPrintTime >= 100) {
    gy25_lastPrintTime = millis();
    Serial.println("GY25 horizontal_angle: 0");
    // Serial.print(gy25.horizontal_angle);
    Serial.print("  ENC1: ");
    Serial.print(Robot.enc_A);
    Serial.print("  ENC2: ");
    Serial.print(Robot.enc_B);
    // testEnc();
    Serial.println();
  }
}

void sendDataToSerial() {
  Serial.println();
  Serial.println("GY25: 0 0 0 0");
  Serial.print("ENC: ");
  Serial.print(Robot.enc_A);
  Serial.print(" ");
  Serial.println(Robot.enc_B);
  Serial.println("VOLTAGE: 0 0");
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
    if      (data.mode=='m' && data.data_counter==2 && data.data_1==1) Robot.motor_A(data.data_2);
    if      (data.mode=='m' && data.data_counter==2 && data.data_1==2) Robot.motor_B(data.data_2);
    else if (data.mode=='M' && data.data_counter==2) Robot.motors(data.data_1, data.data_2);
    else if (data.mode=='F' && data.data_counter==1) forward(data.data_1);
    else if (data.mode=='R' && data.data_counter==1) right(data.data_1);
    else if (data.mode=='L' && data.data_counter==1) left(data.data_1);
    else if (data.mode=='g' && data.data_counter==0) sendDataToSerial();
    else Serial.println("ERROR: unknow command");
  }
  
  //delay(100); // Небольшая задержка между опросами
}