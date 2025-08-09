#ifndef ENCODER_TIMER_H
#define ENCODER_TIMER_H


#include <esp_timer.h>


// #define QUAD_MODE


class EncoderTimer {
public:
    EncoderTimer(int interruptPin, int directionPin, int timerId, uint16_t intervalMicros=1000)
        : interruptPin(interruptPin), directionPin(directionPin), timerId(timerId), intervalMicros(intervalMicros) {}

    void begin() {
        pinMode(interruptPin, INPUT_PULLUP);
        pinMode(directionPin, INPUT_PULLUP);

        const esp_timer_create_args_t timer_args = {
            .callback = &EncoderTimer::onTimer,
            .arg = this,
            .name = "encoder_timer"
        };

        esp_timer_create(&timer_args, &espTimer);
        esp_timer_start_periodic(espTimer, intervalMicros);
    }
    #ifndef QUAD_MODE
    // Define your encoder logic here
    void process_wheel() {
        if(pre_stt != digitalRead(interruptPin)){
            if(pre_stt) encoderCount += !digitalRead(directionPin) ? 1 : -1;
            pre_stt = !pre_stt;
        }
    }
    #else
    void process_wheel() {
        // Read current state
        uint8_t A = digitalRead(interruptPin);
        uint8_t B = digitalRead(directionPin);
        uint8_t currState = (A << 1) | B;

        int8_t delta = decodeQuadrature(prevState, currState);
        encoderCount += delta;

        prevState = currState;
    }

    // Lookup table method for state transition
    int8_t decodeQuadrature(uint8_t prev, uint8_t curr) {
        // State transition table: [prev<<2 | curr] → movement
        const int8_t table[16] = {
            0,  -1,   1,   0,
            1,   0,   0,  -1,
            -1,   0,   0,   1,
            0,   1,  -1,   0
        };

        return table[(prev << 2) | curr];
    }

    #endif

    long getCount() {
        return encoderCount;
    }

    void reset(){
        encoderCount = 0;
    }

private:
    bool pre_stt = false;
    uint8_t prevState = 0;
    volatile long encoderCount = 0;
    int interruptPin;
    int directionPin;
    int timerId;
    uint16_t intervalMicros;
    esp_timer_handle_t espTimer;

    static void onTimer(void* arg) {
        static_cast<EncoderTimer*>(arg)->process_wheel();
    }
};


#endif