// Controller: ESP32 LOLIN32 LITE
// Receive command from mainboard or Bluetooth
// Process to control BLDC motor / BLDC Servo (with encoder)
// Process PID to keep straight forward with signal from Hi229
// 2 BLDC channel with AB encoder and 2 BLDC channel with plush encoder
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;


// #define ROBOT_1


#include "BLDC_motor.h"
#include <EncoderTimer.h>
#include <Encoder.h>
#include <PID_Control.h>
#include <BLDC_servo.h>


//           pwm, dir, brake, speed, dir, brake
BLDC_Motor motor_left( 16,  5, 17, 0, 1, 1);
BLDC_Motor motor_right(22, 23, 19, 0, 0, 1);

BLDC_Motor  slider_motor(27, 26, 14, 1, 0, 1);
Encoder     slider_encoder(34, 35);
PIDController   slider_pid(0.9, 0, 0.003, 200);   // P, I, D, max_speed

#ifdef ROBOT_1
    #define ROBOT_NAME "BLDC_1"
    #define MAX_HEIGHT  400
    BLDC_Servo  slider_servo(slider_motor, slider_encoder, slider_pid, 50);    // steps/mm
#else
    #define ROBOT_NAME "BLDC_2"
    #define MAX_HEIGHT  420
    BLDC_Servo  slider_servo(slider_motor, slider_encoder, slider_pid, 25);    // steps/mm
#endif


bool servo_enable = false;
int current_height = 0;

int wheel_speed = 0;
void setup() {
    Serial.begin(115200);
    SerialBT.begin(ROBOT_NAME); // Set the Bluetooth device name
    Serial2.begin(9600, SERIAL_8N1, 2, 15); // RX, TX

    slider_encoder.begin();

    motor_left.stop();
    motor_right.stop();
    slider_motor.stop();

    delay(1000);
    Serial.println("Started");
}


void forward_command(String command){
    Serial2.println(command);
}


void show_encoder(){
    static long next_update = millis();
    static long prev_pos = 0;

    if(millis() > next_update){
        long current_pos = slider_encoder.getCount();
        String message = String(current_pos-prev_pos) + " \t" + String(current_pos);
        Serial.println(message);

        prev_pos = current_pos;
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
    static String command_1 = "";
    static String command_BT = "";

    // Check for serial commands
    if (Serial.available()) {
        char ch = Serial.read();
        if(ch == '\n'){
            processSerialCommand(command_0);
            command_0 = "";
        }else   command_0 += ch;
    }

    // // Check for serial commands
    // if (Serial1.available()) {
    //     char ch = Serial1.read();
    //     if(ch == '\n'){
    //         processSerialCommand(command_1);
    //         command_1 = "";
    //     }else   command_1 += ch;
    // }


    // Check for serial commands
    if (SerialBT.available()) {
        char ch = SerialBT.read();
        if(ch == '\n'){
            processSerialCommand(command_BT);
            command_BT = "";
        }else   command_BT += ch;
    }
}

void loop() {
    signal_receriver();
    update_servo();
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

        current_height = constrain(current_height+delta, 0, MAX_HEIGHT);
        slider_servo.goto_position_mm(current_height);
    }else{
        if(ch == '0')   slider_motor.stop();
        if(ch == '*')   slider_motor.setSpeed(255);
        if(ch == '/')   slider_motor.setSpeed(-200);
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
        current_height = 0;
        Serial.println("Reset hand at height is 0");
    }
}


void process_hand_servo(int value){
    if(value < 0)   return;
    if(value > MAX_HEIGHT) return;

    if(servo_enable){
        current_height = value;
        slider_servo.goto_position_mm(current_height);
    }
}


#define ball_height 0
void auto_take_ball(){
    // Take ball with 500ms
    long timeout = millis() + 500;
    slider_servo.goto_position_mm(ball_height);
    while(millis() < timeout){
        update_servo();
    }

    // Go up with 20mm
    slider_servo.goto_position_mm(ball_height + 20);
}

#define fire_height 20
void auto_take_fire(){
    // Take fire with 500ms
    long timeout = millis() + 500;
    slider_servo.goto_position_mm(fire_height);
    while(millis() < timeout){
        update_servo();
    }

    // Go up with 20mm
    slider_servo.goto_position_mm(fire_height + 10);
}
void prepare_take_fire(){
    slider_servo.goto_position_mm(fire_height+10);
    forward_command("O01");
}

#define box_height 100
void auto_take_box(){
    // Take fire with 500ms
    long timeout = millis() + 500;
    slider_servo.goto_position_mm(box_height);
    while(millis() < timeout){
        update_servo();
    }

    // Go up with 20mm
    slider_servo.goto_position_mm(box_height + 40);
    forward_command("O11");
}
void prepare_take_box(){
    slider_servo.goto_position_mm(box_height+30);
    forward_command("O01");
}


void process_vaccum(char ch){
    if(ch == '0')   forward_command("OA0");
    if(ch == '1')   forward_command("OA1");
}

void process_combo(int value){
    if(value == 11) prepare_take_box();
    if(value == 12) auto_take_box();

    if(value == 31) prepare_take_fire();
    if(value == 32) auto_take_fire();
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