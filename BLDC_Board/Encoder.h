#ifndef ENCODER_H
#define ENCODER_H

#include "attachInterruptEx.h"


class Encoder {
public:
    Encoder(int interruptPin, int directionPin, int interval=0)
        : pinA(interruptPin), pinB(directionPin), interval(interval) {
        }
    void begin(){
        pinMode(pinA, INPUT_PULLUP);
        pinMode(pinB, INPUT_PULLUP);
        attachInterruptEx(pinA, [this] { this -> ISR(); }, FALLING);
        Serial.println("Started");
    }
    long getCount() {
        return encoderCount;
    }

private:
    int pinA;
    int pinB;
    int interval;
    volatile long encoderCount = 0;
    long next_count;

    void ISR() {
        if(interval){
            if(millis() > next_count){
                encoderCount += !digitalRead(pinB) ? 1 : -1;
                next_count = millis() + interval;
            }
        }else   encoderCount += !digitalRead(pinB) ? 1 : -1;
    }
};


#endif
