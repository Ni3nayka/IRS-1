#ifndef MY_SERVO_H
#define MY_SERVO_H

#include <Arduino.h>

#define MIN_PULSE_WIDTH 544     // Минимальная длительность импульса в микросекундах
#define MAX_PULSE_WIDTH 2400    // Максимальная длительность импульса в микросекундах
#define REFRESH_INTERVAL 20000  // Период обновления сервоприводов (20ms)
#define STEP_DELAY 10           // Задержка между шагами плавного движения

class MyServo {
private:
    struct Servo {
        uint8_t pin;
        uint16_t targetPulse;
        uint16_t currentPulse;
        unsigned long lastUpdate;
    };
    
    Servo* servos;
    uint8_t servoCount;
    bool initialized;
    
    // Вспомогательная функция для преобразования значений
    uint16_t mapAngle(uint8_t angle) {
        return (uint16_t)((long)angle * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) / 180 + MIN_PULSE_WIDTH);
    }
    
public:
    MyServo() : servos(nullptr), servoCount(0), initialized(false) {}
    
    ~MyServo() {
        if (servos != nullptr) {
            delete[] servos;
        }
    }
    
    void setupServo(uint8_t pins[], uint8_t count) {
        if (servos != nullptr) {
            delete[] servos;
        }
        
        servoCount = count;
        servos = new Servo[servoCount];
        
        for (uint8_t i = 0; i < servoCount; i++) {
            servos[i].pin = pins[i];
            servos[i].targetPulse = 1500; // Среднее положение
            servos[i].currentPulse = 1500;
            servos[i].lastUpdate = 0;
            
            pinMode(pins[i], OUTPUT);
            digitalWrite(pins[i], LOW);
        }
        
        initialized = true;
    }
    
    void servoWrite(uint8_t num, uint8_t angle) {
        if (!initialized || num >= servoCount) return;
        
        // Преобразуем угол (0-180°) в длительность импульса
        servos[num].targetPulse = mapAngle(angle);
    }
    
    void servoUpdate() {
        if (!initialized) return;
        
        static unsigned long lastRefresh = 0;
        unsigned long currentTime = micros();
        
        // Обновляем все сервоприводы каждые 20ms
        if (currentTime - lastRefresh >= REFRESH_INTERVAL) {
            lastRefresh = currentTime;
            
            for (uint8_t i = 0; i < servoCount; i++) {
                // Плавное движение к целевой позиции
                if (servos[i].currentPulse != servos[i].targetPulse) {
                    if (servos[i].currentPulse < servos[i].targetPulse) {
                        servos[i].currentPulse++;
                    } else {
                        servos[i].currentPulse--;
                    }
                }
                
                // Генерируем импульс
                digitalWrite(servos[i].pin, HIGH);
                delayMicroseconds(servos[i].currentPulse);
                digitalWrite(servos[i].pin, LOW);
                
                // Небольшая задержка для стабильности
                delayMicroseconds(STEP_DELAY);
            }
        }
    }
    
    // Дополнительные методы для удобства
    uint8_t getServoCount() {
        return servoCount;
    }
    
    bool isInitialized() {
        return initialized;
    }
};

// Глобальный экземпляр для использования
MyServo ServoController;

#endif