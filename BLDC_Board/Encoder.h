#ifndef ENCODER_H
#define ENCODER_H

#include "attachInterruptEx.h"

enum class EncoderType {
    CountOnly,
    AB
};

class Encoder {
public:
    // Count-only: single pulse input, increments on each interrupt
    Encoder(int countPin)
        : pinA(countPin), pinB(-1), interval(0), type(EncoderType::CountOnly) {}

    // AB: interrupt on pinA, direction from pinB
    Encoder(int interruptPin, int directionPin, int interval = 0)
        : pinA(interruptPin), pinB(directionPin), interval(interval), type(EncoderType::AB) {}

    void begin() {
        pinMode(pinA, INPUT_PULLUP);
        if (type == EncoderType::AB) {
            pinMode(pinB, INPUT_PULLUP);
        }
        attachInterruptEx(pinA, [this] { this->ISR(); }, FALLING);
        Serial.println("Started");
    }

    long getCount() {
        return encoderCount;
    }

    void reset() {
        encoderCount = 0;
    }

private:
    int pinA;
    int pinB;
    int interval;
    EncoderType type;
    volatile long encoderCount = 0;
    long next_count;

    void ISR() {
        if (type == EncoderType::CountOnly) {
            encoderCount++;
            return;
        }

        if (interval) {
            if (millis() > next_count) {
                encoderCount += !digitalRead(pinB) ? 1 : -1;
                next_count = millis() + interval;
            }
        } else {
            encoderCount += !digitalRead(pinB) ? 1 : -1;
        }
    }
};

#endif
