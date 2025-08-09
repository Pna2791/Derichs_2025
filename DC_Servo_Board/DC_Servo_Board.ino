// Controller: ARDUINO NANO
// Receive command from mainboard
// Process to control 3 DC Motor (with encoder)
// Process PID to keep straight forward with signal from Hi229
// 3 AB encoder and 1 I2C port


#include "Motor.h"



Motor motor_left(4, 5);
Motor motor_right(7, 6);
int wheel_speed = 0;


void setup() {
    Serial.begin(9600);

    motor_left.stop();
    motor_right.stop();
}


void loop() {
    signal_receriver();
}


void signal_receriver(){
    static String command_0 = "";

    // Check for serial commands
    if (Serial.available()) {
        char ch = Serial.read();
        if(ch == '\n'){
            processSerialCommand(command_0);
            command_0 = "";
        }else   command_0 += ch;
    }
}



void move_wheel(int dir){
    Serial.println("Move " + String(dir));
    if(dir == 0){
        motor_left.setSpeed(0);
        motor_right.setSpeed(0);
        return;
    }
    if(dir == 1){
        motor_left.setSpeed(wheel_speed);
        motor_right.setSpeed(wheel_speed);
        return;
    }
    if(dir == 2){
        motor_left.setSpeed(-wheel_speed);
        motor_right.setSpeed(-wheel_speed);
        return;
    }
    if(dir == 3){
        motor_left.setSpeed(-wheel_speed);
        motor_right.setSpeed(wheel_speed);
        return;
    }
    if(dir == 4){
        motor_left.setSpeed(wheel_speed);
        motor_right.setSpeed(-wheel_speed);
        return;
    }
    if(dir == 5){
        motor_left.setSpeed(0);
        motor_right.setSpeed(wheel_speed);
        return;
    }
    if(dir == 6){
        motor_right.setSpeed(0);
        motor_left.setSpeed(wheel_speed);
        return;
    }
    if(dir == 7){
        motor_right.setSpeed(0);
        motor_left.setSpeed(-wheel_speed);
        return;
    }
    if(dir == 8){
        motor_left.setSpeed(0);
        motor_right.setSpeed(-wheel_speed);
        return;
    }
}



void processSerialCommand(String command) {
    Serial.println(command);
    command.trim();  // Remove any leading/trailing whitespace
    char prefix = command.charAt(0);

    if(prefix == 'S'){  // Chassis speed
        int value = command.substring(2).toInt();
        wheel_speed = value;
        return;
    }

    if(prefix == 'M'){  // Chassis direction
        int value = command.substring(1).toInt();
        move_wheel(value);
        return;
    }
}