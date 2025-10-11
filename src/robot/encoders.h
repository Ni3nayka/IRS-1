
#pragma once

// Пины энкодеров (аппаратные прерывания)
#define ENC1_A 21
#define ENC1_B 20
#define ENC2_A 18
#define ENC2_B 19

volatile long enc1_count = 0;
volatile long enc2_count = 0;

int encoder1_a_old = 0;
int encoder1_b_old = 0;
int encoder2_a_old = 0;
int encoder2_b_old = 0;

int enc_update(int a0, int b0, int a1, int b1) {
	if (!a0 && !b0 &&  a1 && !b1) return 1;
	if ( a0 && !b0 &&  a1 &&  b1) return 1;
	if ( a0 &&  b0 && !a1 &&  b1) return 1;
	if (!a0 &&  b0 && !a1 && !b1) return 1;
	if (!a0 && !b0 && !a1 &&  b1) return -1;
	if (!a0 &&  b0 &&  a1 &&  b1) return -1;
	if ( a0 &&  b0 &&  a1 && !b1) return -1;
	if ( a0 && !b0 && !a1 && !b1) return -1;
	return 0;

}

// Прерывания для энкодеров
void enc1A_ISR() {
	int a = digitalRead(ENC1_A);
	int b = digitalRead(ENC1_B);
	enc1_count += enc_update(encoder1_a_old,encoder1_b_old,a,b);
	encoder1_a_old = a;
	encoder1_b_old = b;
}
void enc1B_ISR() {
	int a = digitalRead(ENC1_A);
	int b = digitalRead(ENC1_B);
	enc1_count += enc_update(encoder1_a_old,encoder1_b_old,a,b);
	encoder1_a_old = a;
	encoder1_b_old = b;
}
void enc2A_ISR() {
	int a = digitalRead(ENC2_A);
	int b = digitalRead(ENC2_B);
	enc2_count += enc_update(encoder2_a_old,encoder2_b_old,a,b);
	encoder2_a_old = a;
	encoder2_b_old = b;
}
void enc2B_ISR() {
	int a = digitalRead(ENC2_A);
	int b = digitalRead(ENC2_B);
	enc2_count += enc_update(encoder2_a_old,encoder2_b_old,a,b);
	encoder2_a_old = a;
	encoder2_b_old = b;
}

// Инициализация энкодеров и прерываний
void setupEncoders() {
	pinMode(ENC1_A, INPUT_PULLUP);
	pinMode(ENC1_B, INPUT_PULLUP);
	pinMode(ENC2_A, INPUT_PULLUP);
	pinMode(ENC2_B, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(ENC1_A), enc1A_ISR, CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENC1_B), enc1B_ISR, CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENC2_A), enc2A_ISR, CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENC2_B), enc2B_ISR, CHANGE);
}

void resetEncoders() {
	enc1_count = 0;
	enc2_count = 0;
}

void printEncoders() {
	Serial.print("Энкодер 1: ");
	Serial.println(enc1_count);
	Serial.print("Энкодер 2: ");
	Serial.println(enc2_count);
}
