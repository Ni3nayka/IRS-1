#include "MPU6050.h"

#define TIME_OUT 20

MPU6050 accgyro;
unsigned long int t1;

void setup() {
    Serial.begin(9600);
    accgyro.initialize();
}

void loop() {
    long int t = millis();
    if( t1 < t ){
        int16_t ax, ay, az, gx, gy, gz;

        t1 = t + TIME_OUT;
        accgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        Serial.println(ay);
    }
}