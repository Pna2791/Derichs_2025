#ifndef WHEEL_H
#define WHEEL_H


#include <PID_Control.h>
#define FW_SPEED    255
#define TURN_SPEED  255


class Motor {
    private:
        int dir_pin, speed_pin, pre_speed, direction;
    
    public:
        Motor(int dir_pin, int speed_pin, int direction=0){
            this->dir_pin   = dir_pin;
            this->speed_pin = speed_pin;
            this->direction = direction;

            pinMode(dir_pin, OUTPUT);
            pinMode(speed_pin, OUTPUT);
        }
    
        void setSpeed(int val) {
            if (val == 0) {
                digitalWrite(dir_pin, pre_speed>0);
                analogWrite(speed_pin, 0);
                return;
            }

            digitalWrite(dir_pin, val<0);
            analogWrite(speed_pin, abs(val));
            pre_speed = val;
        }

        void stop(){
            setSpeed(0);
        }
};


class Chassis {
    private:
        Motor &left_motor;
        Motor &right_motor;
        PIDController &forward_pid;

        int left_speed, right_speed;
        long status_index = 0;
        int soft_start_duration = 1;
        int chassis_speed = 0;
        int target_dir = 0;


    public: 
        int code = 0;

        Chassis(Motor &left_motor, Motor &right_motor, PIDController &forward_pid) 
            : left_motor(left_motor), right_motor(right_motor), forward_pid(forward_pid){}

        void move_speed(int val){
            left_motor.setSpeed(val);
            right_motor.setSpeed(val);
        }

        void rotate_CCW(int val){
            left_motor.setSpeed(-val);
            right_motor.setSpeed(val);
        }

        void stop(){
            left_speed = 0;
            right_speed = 0;
            code = 0;
            move_speed(0);
        }

        void move_forward(float delta, int speed=0){
            if(delta == 0){
                left_speed = -speed;
                right_speed = -speed;
            }else if(delta > 0){
                left_speed = -constrain(speed-delta*speed, 1, 255);
                right_speed = -speed;
            }else{
                left_speed = -speed;
                right_speed = -constrain(speed+delta*speed, 1, 255);
            }
        }
        
        void set_speed(int left, int right, int softstart=1){
            left_speed = left;
            right_speed = right;
            soft_start_duration = softstart;
            status_index = 0;
        }

        void set_target_dir(int dir){
            target_dir = dir;
        }

};


#endif