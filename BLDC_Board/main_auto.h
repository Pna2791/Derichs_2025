#ifndef MAIN_AUTO_H
#define MAIN_AUTO_H

#include "config.h"


// Receive command from mainboard or Bluetooth
// Process to control BLDC motor / BLDC Servo (with encoder)
// Process PID to keep straight forward with signal from Hi229
// 2 BLDC channel with AB encoder and 2 BLDC channel with plush encoder
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;


#define ROBOT_NAME "BLDC_1"


#include "BLDC_motor.h"
#include <EncoderTimer.h>
#include <Encoder.h>
#include <PID_Control.h>
#include <BLDC_servo.h>


Encoder     left_encoder(13);
Encoder     right_encoder(18);

BLDC_Motor  slider_motor(27, 26, 14, 1, 0, 1);
Encoder     slider_encoder(34, 35);

//           pwm, dir, brake, speed, dir, brake
BLDC_Motor motor_left( 16,  5, 17, 1, 0, 1);
BLDC_Motor motor_right(22, 23, 19, 1, 1, 1);

// PIDController   slider_pid(4, 0.004, 0.01, -150, 255, 20);   // P, I, D, max_speed
PIDController   slider_pid(2, 0.002, 0.01, -150, 255, 20);   // P, I, D, max_speed


#define MAX_HEIGHT 300
BLDC_Servo slider_servo(slider_motor, slider_encoder, slider_pid, 50); // steps/mm


bool servo_enable = false;

int wheel_speed = 0;

void processSerialCommand(String command);

void setup() {
    Serial.begin(115200);
    SerialBT.begin(ROBOT_NAME); // Set the Bluetooth device name
    Serial2.begin(9600, SERIAL_8N1, 2, 15); // RX, TX

    slider_encoder.begin();
    left_encoder.begin();
    right_encoder.begin();
    slider_servo.limit(0, MAX_HEIGHT);

    motor_left.stop();
    motor_right.stop();
    slider_motor.stop();

    delay(1000);
    Serial.println("Started");
}


void forward_command(String command){
    Serial2.println(command);
    Serial.println(command);
}


void show_encoder(){
    static long next_update = millis();
    static long prev_pos = 0;

    if(millis() > next_update){
        long slider_pos = slider_encoder.getCount();
        long left_pos = left_encoder.getCount();
        long right_pos = right_encoder.getCount();
        String message = String(slider_pos - prev_pos) + " \t" + String(slider_pos)
                       + " \t" + String(left_pos) + " \t" + String(right_pos);
        Serial.println(message);

        prev_pos = slider_pos;
        next_update += 1000;
    }
}


void update_servo(){
    static long next_update = millis();
    if(millis() > next_update){
        if(servo_enable){
            slider_servo.run();
            show_encoder();
        }

        next_update += INTERVAL;
    }
}


void signal_receriver(){
    static String command_0 = "";
    static String command_2 = "";
    static String command_BT = "";

    // Check for serial commands
    if (Serial.available()) {
        char ch = Serial.read();
        if(ch == '\n'){
            String command = command_0;
            command_0 = "";
            processSerialCommand(command);
        }else   command_0 += ch;
    }

    // Check for serial commands
    if (Serial2.available()) {
        char ch = Serial2.read();
        if(ch == '\n'){
            String command = command_2;
            command_2 = "";
            processSerialCommand(command);
        }else   command_2 += ch;
    }


    // Check for serial commands
    if (SerialBT.available()) {
        char ch = SerialBT.read();
        if(ch == '\n'){
            String command = command_BT;
            command_BT = "";
            processSerialCommand(command);
        }else   command_BT += ch;
    }
}

void loop() {
    signal_receriver();
    update_servo();
}

void my_delay(int value){
    long time_out = millis() + value;
    while(millis() < time_out){
        signal_receriver();
        update_servo();
    }
}


void check_servo(int speed){
    servo_enable = false;
    slider_motor.setSpeed(speed);
    my_delay(5);
    servo_enable = true;
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


void process_hand(char ch){
    if(servo_enable){
        int delta = 0;
        if(ch == '*')   delta =  50;
        if(ch == '/')   delta = -50;
        if(ch == '+')   delta =  5;
        if(ch == '-')   delta = -5;
        if(delta > 0){
            slider_motor.setSpeed(-10);
            my_delay(1);
        }else{
            slider_motor.setSpeed(10);
            my_delay(1);
        }
        slider_servo.move_position_mm(delta);
    }else{
        if(ch == '0')   slider_motor.stop();
        if(ch == '*')   slider_motor.setSpeed(255);
        if(ch == '/')   slider_motor.setSpeed(-170);
        if(ch == '+')   slider_motor.setSpeed(100);
        if(ch == '-')   slider_motor.setSpeed(-80);
    }
    if(ch == 'E')   servo_enable = true;
    if(ch == 'D'){
        servo_enable = false;
        slider_motor.stop();
    }
    if(ch == 'R'){
        slider_servo.hard_reset();
        Serial.println("Reset hand at height is 0");
    }
}


void process_hand_servo(int value){
    if(value < 0)   return;
    if(value > MAX_HEIGHT) return;

    if(servo_enable){
        slider_servo.goto_position_mm(value);
    }
}


void auto_reset(){
    Serial.println("Auto reset");

    servo_enable = false;
    #if defined(ROBOT_NAP_2)
        slider_motor.setSpeed(-20);
        my_delay(4000);
    #else
        slider_motor.setSpeed(-30);
        my_delay(1000);
    #endif
    slider_motor.stop();
    my_delay(500);

    slider_servo.reset(-10);
    servo_enable = true;
}


#define auto_take_delay 1000
// NAP's Combo
void prepare_D30(){
    forward_command("OA1");
    slider_servo.goto_position_mm(50);
}
void take_D30(){
    check_servo(20);
    slider_servo.goto_position_mm(0);
    my_delay(auto_take_delay);
    
    slider_servo.goto_position_mm(100);
}

void prepare_D40(){
    forward_command("OA1");
    slider_servo.goto_position_mm(150);
}
void take_D40(){
    check_servo(20);
    slider_servo.goto_position_mm(100);
    my_delay(auto_take_delay);
    
    slider_servo.goto_position_mm(200);
}


void process_vaccum(char ch){
    if(ch == '0')   forward_command("OA0");
    if(ch == '1')   forward_command("OA1");
}


void process_combo(int value){
    if(value == 0) auto_reset();
    if(value == 11) prepare_D30();
    if(value == 12) take_D30();
    if(value == 13) prepare_D40();
    if(value == 14) take_D40();

    if(value == 16) forward_command("OA0");
    if(value == 17) forward_command("O21");
}


void processSerialCommand(String command) {
    Serial.println(command);
    command.trim();  // Remove any leading/trailing whitespace
    char prefix = command.charAt(0);

    if(prefix == 'S'){  // Chassis speed
        forward_command(command);
        int value = command.substring(2).toInt();
        wheel_speed = value;
        return;
    }

    if(prefix == 'M'){  // Chassis direction
        forward_command(command);
        int value = command.substring(1).toInt();
        move_wheel(value);
        return;
    }

    if(prefix == 'H'){  // Chassis direction
        int value = command.substring(1).toInt();
        process_hand_servo(value);
        return;
    }

    if(prefix == 'C'){  // Chassis direction
        int value = command.substring(1).toInt();
        process_combo(value);
        return;
    }

    if(prefix == 'B'){  // Body
        process_hand(command.charAt(1));
        return;
    }

    if(prefix == 'O'){  // ON/OFF
        if(command.length() == 3)
            forward_command(command);
        return;
    }

    if(prefix == 'T'){  // ON/OFF
        process_vaccum(command.charAt(1));
        return;
    }


}

#endif