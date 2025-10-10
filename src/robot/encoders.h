// Пины энкодеров (аппаратные прерывания)
#define ENC1_A 18
#define ENC1_B 19
#define ENC2_A 20
#define ENC2_B 21

volatile long enc1_count = 0;
volatile long enc2_count = 0;

// Прерывания для энкодеров
void enc1A_ISR() {
	if (digitalRead(ENC1_B) == HIGH) enc1_count++;
	else enc1_count--;
}
void enc1B_ISR() {
	if (digitalRead(ENC1_A) == HIGH) enc1_count--;
	else enc1_count++;
}
void enc2A_ISR() {
	if (digitalRead(ENC2_B) == HIGH) enc2_count++;
	else enc2_count--;
}
void enc2B_ISR() {
	if (digitalRead(ENC2_A) == HIGH) enc2_count--;
	else enc2_count++;
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
