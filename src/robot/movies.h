// Файл для функций движения робота
#pragma once
#include "BTS7960_PRO.h"
#include <Servo.h>
// #include "gy-25.h"

#define MOSFET_PIN 8

// BTS7960_PRO Motors;

// extern BTS7960_PRO Motors;
// extern Servo servos[];
// extern int currentPositions[];
// extern GY25 gy25;
// extern const uint8_t servoCount;

// #define ENC_POROG 50
// #define ENC_TIME 500
// #define ENC_GYRO_TURN_POROG 3
// #define ENC_FORWARD_KP 0.5
// #define ENC_FORWARD_KD 10
// #define ENC_FORWARD_ALIGNMENT_KP 20
// #define ENC_TURN_KP 5.0
// #define ENC_TURN_KD 4
// #define ENC_GYRO_FORWARD_KP 15
// #define GYRO_TURN_KP 5
// #define GYRO_TURN_KD 40
// #define ENC_ANGLE_TO_PARROT 17
// #define ENC_CM_TO_PARROT 130
// #define ENC_MOTOR_MAX_SPEED 70
// #define ENC_MOTOR_MAX_SPEED_TURN 45
// #define ENC_MOTOR_R_BOOST 1.07

// extern volatile long enc1_count;
// extern volatile long enc2_count;


// Массив пинов сервоприводов
#define SERVO_QUANTITY 2
const uint8_t servoPins[SERVO_QUANTITY] = {10, 11};
const uint8_t servoCount = sizeof(servoPins) / sizeof(servoPins[0]);
Servo servos[SERVO_QUANTITY];
int currentPositions[SERVO_QUANTITY] = {90, 90};


// Подключаем файл с энкодерами
#include "encoders.h"
#define GY25_SERIAL Serial2
#include "gy-25.h" // Подключаем библиотеку гироскопа
// Экземпляр гироскопа (указываем RX и TX пины)
GY25 gy25; // (12, 8);
unsigned long int gy25_lastPrintTime = 0;

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

// Объявление объекта управления моторами
BTS7960_PRO Motors;


void runGyro(long int forward = 0);
void turnGyro(long int right = 0);
void smoothMoveServo(int servoNum, int targetAngle, int speed = 35);

long int enc_strafe = 0;
unsigned long int enc_strafe_timer = 0;
#define GYRO_STRAFE_DT 1500 // 2100
#define GYRO_STRAFE_ANDLE -1 // подруливать в: 1 - вправо, -1 - влево

void updateGyroStrafe() {
	if (enc_strafe_timer<millis()) {
		enc_strafe += GYRO_STRAFE_ANDLE;
		enc_strafe_timer = millis() + GYRO_STRAFE_DT;
	}
}

long int getGyroStrafe() {
	gy25.update();
	updateGyroStrafe();
	return gy25.horizontal_angle+enc_strafe;
}

// Реализация функций
void runGyro(long int forward) {
	long int enc_target = enc1_count + forward * ENC_CM_TO_PARROT;
	long int e_old = 0;
	long int gyro_target = getGyroStrafe(); //  + 5
	Motors.run(1, 100);
	Motors.run(2, 100);
	delay(400);
	long int time = millis() + ENC_TIME;
	enc_strafe_timer = millis() + GYRO_STRAFE_DT;
	while (time > millis()) {
		// gy25.update();
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
		long int e_gyro = getGyroStrafe() - gyro_target;
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
}

void turnGyro(long int right) {
	long int time = millis() + ENC_TIME;
	long int e_old = 0;
	long int gyro_target = getGyroStrafe() - right; //  + 5
	enc_strafe_timer = millis() + GYRO_STRAFE_DT;
	while (time > millis()) {
		// gy25.update();
		if ((abs(getGyroStrafe() - gyro_target) > ENC_GYRO_TURN_POROG)) {
			time = millis() + ENC_TIME;
		}
		long int e = getGyroStrafe() - gyro_target;
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
}

void smoothMoveServo(int servoNum, int targetAngle, int speed) {
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
