#ifndef BLDC_MOTOR_H
#define BLDC_MOTOR_H

#include <Arduino.h>

class BLDC_Motor {
private:
    uint8_t speed_pin;
    uint8_t dir_pin;
    uint8_t brake_pin;
    bool speed_state;
    bool dir_state;
    bool brake_state;

public:
    BLDC_Motor(uint8_t speed_pin, uint8_t dir_pin, uint8_t brake_pin, bool speed_state, bool dir_state, bool brake_state)
        : speed_pin(speed_pin), dir_pin(dir_pin), brake_pin(brake_pin), speed_state(speed_state), dir_state(dir_state), brake_state(brake_state) {
        pinMode(speed_pin, OUTPUT);
        pinMode(dir_pin, OUTPUT);
        pinMode(brake_pin, OUTPUT);
    }

    void setSpeed(int speed) {
        if (speed == 0) {
            setBrake(true);
        } else {
            setBrake(false);
        }
        bool direction = speed >= 0;
        uint8_t pwm = speed_state ? abs(speed) : (255 - abs(speed));
        analogWrite(speed_pin, pwm);
        setDirection(direction);
    }

    void stop() {
        setSpeed(0);
    }

    void setDirection(bool direction) {
        digitalWrite(dir_pin, direction == dir_state ? HIGH : LOW);
    }

    void setBrake(bool brake) {
        digitalWrite(brake_pin, brake == brake_state ? HIGH : LOW);
    }
};

#endif // BLDC_MOTOR_H
