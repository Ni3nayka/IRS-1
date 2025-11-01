// Файл для функций движения робота
#pragma once

#include "BTS7960_PRO.h"
#include <Servo.h>

// Массив пинов сервоприводов
#define SERVO_QUANTITY 2
const uint8_t servoPins[SERVO_QUANTITY] = {10, 11};
const uint8_t servoCount = sizeof(servoPins) / sizeof(servoPins[0]);
Servo servos[SERVO_QUANTITY];
int currentPositions[SERVO_QUANTITY] = {90, 90};

#define GY25_STRAFE_DT 1500
#define GY25_SERIAL Serial2
#include "gy-25.h"
GY25 gy25; // (12, 8); (указываем RX и TX пины)
unsigned long int gy25_lastPrintTime = 0;

// Подключаем файл с энкодерами
#include "encoders.h"
#define ENC_POROG 50
#define ENC_TIME 500
#define ENC_GYRO_TURN_POROG 3
#define ENC_FORWARD_KP 0.5
#define ENC_FORWARD_KD 10
#define ENC_FORWARD_ALIGNMENT_KP 20 // выравнивание колес друг относительно друга 
#define ENC_TURN_KP 5.0
#define ENC_TURN_KD 4
#define ENC_GYRO_FORWARD_KP 3
#define GYRO_TURN_KP 7 // 8
#define GYRO_TURN_KD 50 // 40
#define ENC_ANGLE_TO_PARROT 18.1 // 17
#define ENC_CM_TO_PARROT 120 //130
#define ENC_MOTOR_MAX_SPEED 70 // 70
#define ENC_MOTOR_MAX_SPEED_TURN 70
#define ENC_MOTOR_R_BOOST 1 //1.07 // ОН ОТВЕЧАЕТ ЗА ЛЕВЫЙ МОТОР!!!
// #define ENC_UPDATE_DELAY_FOR_CHECK_ERROR 1000
// #define ENC_UPDATE_DELTA_FOR_CHECK_ERROR 100

// Объявление объекта управления моторами
BTS7960_PRO Motors;

// ========================= VOLTAGE ==============================

const int voltage_counter = 2;
const int voltage_pins[voltage_counter] = {A0, A1};

float getVoltage(int number) {
	if (number>voltage_counter || number<1) return 0.0;
	return analogRead(voltage_pins[number])*0.029325513;
	// int a = 0;
	// const int iteritions = 3;
	// for (int i = 0; i<iteritions; i++) {
	// 	a += analogRead(voltage_pins[number]);
	// 	delay(100);
	// }
	// return float(a)/iteritions*0.029325513;
	
}

// ========================= SERIAL ==============================

#include "serialParser.h"
void sendDataToSerial() {
  Serial.println();
  Serial.print("GY25: ");
  Serial.print(gy25.angle[0]);
  Serial.print(" ");
  Serial.print(gy25.angle[1]);
  Serial.print(" ");
  Serial.print(gy25.angle[2]);
  Serial.print(" ");
  // Serial.print(gy25.horizontal_angle);
  Serial.print(gy25.horizontal_angle_strafe);
  Serial.println();
  Serial.print("ENC1: ");
  Serial.println(enc1_count);
  Serial.print("ENC2: ");
  Serial.println(enc2_count);
  Serial.print("VOLTAGE1: ");
  Serial.println(getVoltage(1));
  Serial.print("VOLTAGE2: ");
  Serial.println(getVoltage(2));
}

// ========================= CODE ==============================

void setupRobot() {
	Serial.begin(9600);
	Motors.setup();
	gy25.setup();
	// gy25.calibration();
	Serial2.begin(115200); // ПОТОМУ ЧТО ТУПАЯ АРДУИНА 
	for (uint8_t i = 0; i < servoCount; i++) {
		servos[i].attach(servoPins[i]);
		servos[i].write(90);
		gy25.update();
	}
	setupEncoders();
	delay(1000);
	gy25.update();
	Serial.println("System ready!");
}

// Реализация функций
void runGyro(long int forward=0) {
	long int enc_target = enc1_count + forward * ENC_CM_TO_PARROT;
	long int e_old = 0;
	gy25.update();
	long int gyro_target = gy25.horizontal_angle_strafe; //  + 5
	Motors.run(1, 100);
	Motors.run(2, 100);
	delay(400);
	long int time = millis() + ENC_TIME;
	//enc_strafe_timer = millis() + GYRO_STRAFE_DT;
	int serial_e_for_gyro = 0;
	gy25.setupStrafe();
	// long int enc_1_old = enc1_count, enc_2_old = enc2_count, enc_update_timer = millis() + ENC_UPDATE_DELAY_FOR_CHECK_ERROR;
	while (time > millis()) {
		// serial
		SerialData data = readSerialData();
  	if (data.mode != '\0') {
			if      (data.mode=='S' && data.data_counter==0) break;
			else if (data.mode=='e' && data.data_counter==1) serial_e_for_gyro = data.data_1;
			else if (data.mode=='g' && data.data_counter==0) sendDataToSerial();
			else Serial.println("ERROR: unknow command");
		}
		// check error (робот мощный и тупо буксует)
		// if (enc_update_timer<millis()) {
		// 	if (abs(enc_1_old-enc1_count)<ENC_UPDATE_DELTA_FOR_CHECK_ERROR || abs(enc_2_old-enc2_count)<ENC_UPDATE_DELTA_FOR_CHECK_ERROR) {
		// 		Motors.run(1, 0);
		// 		Motors.run(2, 0);
		// 		Serial.println("ERROR: forward - enc stop");
		// 		return;
		// 	}
		// 	enc_update_timer = millis() + ENC_UPDATE_DELAY_FOR_CHECK_ERROR;
		// }
		// move
		gy25.update();
		if ((abs(enc1_count - enc_target) > ENC_POROG)) {
			time = millis() + ENC_TIME;
		}
		long int e = enc_target - enc1_count;
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
		long int e_gyro = gy25.horizontal_angle_strafe - gyro_target + serial_e_for_gyro;
		long int p_gyro = e_gyro * ENC_GYRO_FORWARD_KP;
		long int m1 = constrain(p + d, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED);
		long int m2 = constrain(p + d, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED);
		m1 = constrain(m1 + p_gyro, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED) * ENC_MOTOR_R_BOOST;
		m2 = constrain(m2 - p_gyro, -ENC_MOTOR_MAX_SPEED, ENC_MOTOR_MAX_SPEED);
		Motors.run(1, m1);
		Motors.run(2, m2);
	}
	Motors.run(1, 0);
	Motors.run(2, 0);
	Serial.println("END COMMAND: forward - success");
}

void turnGyro(long int right=0) {
	long int time = millis() + ENC_TIME;
	long int e_old = 0;
	gy25.update();
	long int gyro_target = gy25.horizontal_angle_strafe - right; //  + 5
	//enc_strafe_timer = millis() + GYRO_STRAFE_DT;
	gy25.setupStrafe();
	while (time > millis()) {
		gy25.update();
		if ((abs(gy25.horizontal_angle_strafe - gyro_target) > ENC_GYRO_TURN_POROG)) {
			time = millis() + ENC_TIME;
		}
		long int e = gy25.horizontal_angle_strafe - gyro_target;
		long int p = e;
		long int d = e - e_old;
		e_old = e;
		p *= GYRO_TURN_KP;
		d *= GYRO_TURN_KD;
		long int m1 = constrain(p + d, -ENC_MOTOR_MAX_SPEED_TURN, ENC_MOTOR_MAX_SPEED_TURN);
		long int m2 = -constrain(p + d, -ENC_MOTOR_MAX_SPEED_TURN, ENC_MOTOR_MAX_SPEED_TURN);
		Motors.run(1, m1);
		Motors.run(2, m2);
	}
	Motors.run(1, 0);
	Motors.run(2, 0);
	Serial.println("END COMMAND: turn - success");
}

void smoothMoveServo(int servoNum, int targetAngle, int speed=35) {
	if (servoNum < 0 || servoNum >= SERVO_QUANTITY) {
		return;
	}
	targetAngle = constrain(targetAngle, 0, 180);
	int startAngle = currentPositions[servoNum];
	if (targetAngle > startAngle) {
		for (int angle = startAngle; angle <= targetAngle; angle++) {
			servos[servoNum].write(angle);
			currentPositions[servoNum] = angle;
			delay(speed);
			gy25.update();
		}
	} else {
		for (int angle = startAngle; angle >= targetAngle; angle--) {
			Serial.println(speed);
			servos[servoNum].write(angle);
			currentPositions[servoNum] = angle;
			delay(speed);
			gy25.update();
		}
	}
}

void brushesOn() {
  gy25.delayUpdate(1000);
  Motors.run(4, 100);
  gy25.delayUpdate(1000);
}
void brushesOff() {
  gy25.delayUpdate(1000);
  Motors.run(4, 0);
  gy25.delayUpdate(1000);
}


