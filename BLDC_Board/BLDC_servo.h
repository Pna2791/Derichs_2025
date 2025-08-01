#ifndef BLDC_SERVO_H
#define BLDC_SERVO_H


#include "BLDC_motor.h"
#include <EncoderTimer.h>
#include <Encoder.h>
#include <PID_Control.h>

#define BLDC


class BLDC_Servo{
    private:
        #ifdef BLDC
            Encoder &encoder;
            BLDC_Motor &motor;
        #else
            EncoderTimer &encoder;
            Motor &motor;
        #endif


        PIDController &pid_controller;
        float pluss_per_mm;

        int target_pos, offset;
    
    public:

        #ifdef BLDC
            BLDC_Servo(BLDC_Motor &motor, Encoder &encoder, PIDController &pid_controller, float plush_per_mm=50) 
                : motor(motor), encoder(encoder), pid_controller(pid_controller){
                    this->pluss_per_mm = plush_per_mm;
                }
        #else
            BLDC_Servo(Motor &motor, EncoderTimer &encoder, PIDController &pid_controller, float plush_per_mm=50) 
                : motor(motor), encoder(encoder), pid_controller(pid_controller){
                    this->pluss_per_mm = plush_per_mm;
                }
        #endif
        
        void goto_position_mm(int target){
            goto_position(pluss_per_mm*target);
        }

        void goto_position(int target){
            this->target_pos = offset+target;
            run();
        }
        
        void reset(int value=0){
            int current_pos = encoder.getCount();
            offset = current_pos - pluss_per_mm*value;
            target_pos = current_pos;
        }

        void hard_reset(){
            target_pos = 0;
            offset = 0;
            encoder.reset();
        }

        void stop(){
            motor.stop();
        }

        void run(){
            int current_pos = encoder.getCount();
            int value = pid_controller.compute(target_pos, current_pos);

            #ifdef DEBUG
                Serial.print(target_pos-current_pos);
                Serial.print('\t');
                Serial.println(value);
            #endif

            motor.setSpeed(value);
        }
};







#endif