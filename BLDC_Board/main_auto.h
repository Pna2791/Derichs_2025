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
#include "hi229.h"


Encoder     left_encoder(13);
Encoder     right_encoder(18);

BLDC_Motor  slider_motor(27, 26, 14, 1, 0, 1);
Encoder     slider_encoder(34, 35);

//           pwm, dir, brake, speed, dir, brake
BLDC_Motor motor_left( 16,  5, 17, 1, 0, 1);
BLDC_Motor motor_right(22, 23, 19, 1, 1, 1);

// PIDController   slider_pid(4, 0.004, 0.01, -150, 255, 20);   // P, I, D, max_speed
PIDController   slider_pid(2, 0.002, 0.01, -150, 255, 20);   // P, I, D, max_speed

PIDController   forward_pid(10, 2, 1, -192, 192); // 315rpm speed 100-150
PIDController   rotate_pid(2, 0.0, 0.15, -255, 255); // 315rpm speed 100-150

#define MAX_HEIGHT 300
BLDC_Servo slider_servo(slider_motor, slider_encoder, slider_pid, 50); // steps/mm


bool servo_enable = false;
bool emergency_stop = false;

int target_dir = 0;
int wheel_speed = 0;

void processSerialCommand(String command);

void setup() {
    Serial.begin(115200);
    SerialBT.begin(ROBOT_NAME); // Set the Bluetooth device name
    
    // Serial2 dùng cho cảm biến góc Hi229 (như cũ)
    Serial2.begin(115200, SERIAL_8N1, 2, 15); // RX=2, TX=15
    
    // Serial1 dùng để giao tiếp UART với Mainboard (do Serial2 đã bị chiếm)
    // Cắm dây RX của Mainboard vào chân 32 của ESP32.
    // Cắm dây TX của Mainboard vào chân 33 của ESP32 (nếu cần đọc ngược lại).
    Serial1.begin(9600, SERIAL_8N1, 33, 32); // RX=33, TX=32

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
    Serial.println(command);      // In ra USB để debug
    Serial1.println(command);     // Gửi xuống Mainboard qua Serial1
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
    static String command_1 = "";
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

    // Check for Mainboard commands (nếu Mainboard có phản hồi)
    if (Serial1.available()) {
        char ch = Serial1.read();
        if(ch == '\n'){
            String command = command_1;
            command_1 = "";
            processSerialCommand(command);
        }else   command_1 += ch;
    }

    // Check for Bluetooth commands
    if (SerialBT.available()) {
        char ch = SerialBT.read();
        if(ch == '\n'){
            String command = command_BT;
            command_BT = "";
            processSerialCommand(command);
        }else   command_BT += ch;
    }
}


void my_loop(){
    signal_receriver();
    update_servo();
    // int direction = get_direction(Serial2);
    // if(direction != 0xFFF){
    //     Serial.println("Ang: " + String(direction));
    // }
}


void loop() {
    emergency_stop = false;
    my_loop();
}

void my_delay(int value){
    long time_out = millis() + value;
    while(millis() < time_out){
        my_loop();
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





#define DEBUG

#define WHEEL_DIAMETER 100
#define GEAR_RATIO 14   
#define STEPS_PER_REVOLUTION 6

// gear_ratio * steps_per_revolution / wheel_diameter / pi
const float step_per_mm = 1.0f * GEAR_RATIO * STEPS_PER_REVOLUTION / WHEEL_DIAMETER / 3.1416;

#define brake_distance      10
#define slowdown_distance   250
#define auto_forward_speed  120
void auto_forward(int distance){
    Serial.println("Auto forward: " + String(distance));
    Serial.println("step_per_mm: " + String(step_per_mm));
    forward_pid.reset();
    float delta_plush = step_per_mm * (distance-brake_distance);
    long left_pos = left_encoder.getCount() + delta_plush;
    long right_pos = right_encoder.getCount() + delta_plush;
    Serial.println("Left target pos: " + String(left_pos));
    Serial.println("Right target pos: " + String(right_pos));

    int auto_speed = auto_forward_speed;
    motor_left.setSpeed(auto_speed);
    motor_right.setSpeed(auto_speed);

    bool is_normal_speed = true;
    float delta_slowdown = step_per_mm*slowdown_distance;
    int left_pos_slowdown = left_pos - delta_slowdown;
    int right_pos_slowdown = right_pos - delta_slowdown;
    while(left_encoder.getCount() < left_pos || right_encoder.getCount() < right_pos){
        if (
            is_normal_speed 
            && (left_encoder.getCount() > left_pos_slowdown)
            && (right_encoder.getCount() > right_pos_slowdown)
        ){
            auto_speed = auto_forward_speed * 0.4;
            is_normal_speed = false;
        }

        
        my_loop();
        if(emergency_stop){
            Serial.println("Emergency stopped");
            motor_left.stop();
            motor_right.stop();
            return;
        }
        
        int direction = get_direction(Serial2);
        if(direction != 0xFFF){
            Serial.println("Ang: " + String(direction));
            direction = standard_dir(target_dir, direction);

            float delta_value = forward_pid.compute(target_dir, direction)/255;
            #ifdef DEBUG
                String message = String(delta_value*10) + '\t' + String(target_dir-direction);
                SerialBT.println(message);
            #endif

            motor_left.setSpeed(auto_speed * (1 - delta_value));
            motor_right.setSpeed(auto_speed * (1 + delta_value));
        }
    }

    motor_left.stop();
    motor_right.stop();
    Serial.println("Finish forward: " + String(distance));
    Serial.println("Left current pos: " + String(left_encoder.getCount()));
    Serial.println("Right current pos: " + String(right_encoder.getCount()));
}

void auto_backward(int distance){
    Serial.println("Auto backward: " + String(distance));
    Serial.println("step_per_mm: " + String(step_per_mm));
    forward_pid.reset();
    float delta_plush = step_per_mm * (distance-brake_distance);
    long left_pos = left_encoder.getCount() - delta_plush;
    long right_pos = right_encoder.getCount() - delta_plush;
    Serial.println("Left target pos: " + String(left_pos));
    Serial.println("Right target pos: " + String(right_pos));

    int auto_speed = -auto_forward_speed;
    motor_left.setSpeed(auto_speed);
    motor_right.setSpeed(auto_speed);

    bool is_normal_speed = true;
    float delta_slowdown = step_per_mm*slowdown_distance;
    int left_pos_slowdown = left_pos + delta_slowdown;
    int right_pos_slowdown = right_pos + delta_slowdown;
    
    while(left_encoder.getCount() > left_pos || right_encoder.getCount() > right_pos){
        if (
            is_normal_speed 
            && (left_encoder.getCount() < left_pos_slowdown)
            && (right_encoder.getCount() < right_pos_slowdown)
        ){
            auto_speed = -auto_forward_speed * 0.4;
            is_normal_speed = false;
        }

        my_loop();
        if(emergency_stop){
            Serial.println("Emergency stopped");
            motor_left.stop();
            motor_right.stop();
            return;
        }
        
        int direction = get_direction(Serial2);
        if(direction != 0xFFF){
            Serial.println("Ang: " + String(direction));
            direction = standard_dir(target_dir, direction);

            // Công thức PID này vẫn giữ nguyên, vì delta_value vẫn có dấu phản ứng ngược 
            // giống như tiến. Khi đi lùi (tốc độ âm), bánh trái chạy (âm * (1 - delta)) -> nhanh hơn
            // nếu cần quay đầu về trái, tương đương xoay đuôi về phải, giúp khử góc lệch.
            float delta_value = forward_pid.compute(target_dir, direction)/255;
            #ifdef DEBUG
                String message = String(delta_value*10) + '\t' + String(target_dir-direction);
                SerialBT.println(message);
            #endif

            motor_left.setSpeed(auto_speed * (1 - delta_value));
            motor_right.setSpeed(auto_speed * (1 + delta_value));
        }
    }

    motor_left.stop();
    motor_right.stop();
    Serial.println("Finish backward: " + String(distance));
    Serial.println("Left current pos: " + String(left_encoder.getCount()));
    Serial.println("Right current pos: " + String(right_encoder.getCount()));
}

void auto_run_trajectory() {
    Serial.println("Start blind run trajectory...");
    // Ví dụ mẫu về một quỹ đạo chạy mù:
    // auto_forward(1000);  // Tiến 1m
    // rote_CCW();          // Xoay trái 90 độ
    // auto_forward(500);   // Tiến 0.5m
    // rote_CW();           // Xoay phải 90 độ
    // auto_backward(1000); // Lùi 1m
    
    // Bạn có thể tự thêm các bước chạy cụ thể vào đây.
    Serial.println("End blind run trajectory");
}


#define auto_rotate_speed    150
#define error_angle     10
#define rotate_timeout  3000
void rote_CCW(){
    target_dir += 900;
    if(target_dir > 1800)   target_dir -= 3600;
    
    motor_left.stop();
    motor_right.setSpeed(auto_rotate_speed);
    long time_out = millis() + rotate_timeout;
    while (millis() < time_out){
        my_loop();
        int direction = get_direction(Serial2);
        if(direction != 0xFFF){
            Serial.println("Ang: " + String(direction));
            direction = standard_dir(target_dir, direction);
            if(abs(target_dir - direction) < error_angle){
                motor_right.stop();
                Serial.println("Stoped: " + String(rotate_timeout - time_out + millis()));
                return;
            }

            int rotate_speed = auto_rotate_speed * rotate_pid.compute(target_dir, direction)/255;
            #ifdef DEBUG
                String message = String(rotate_speed) + '\t' + String(target_dir-direction);
                SerialBT.println(message);
            #endif
            motor_right.setSpeed(rotate_speed);
        }
    }
    motor_left.stop();
    motor_right.stop();
}
void rote_CW(){
    target_dir -= 900;
    if(target_dir < -1800)   target_dir += 3600;
    
    motor_right.stop();
    motor_left.setSpeed(auto_rotate_speed);
    long time_out = millis() + rotate_timeout;
    while (millis() < time_out){
        my_loop();
        int direction = get_direction(Serial2);
        if(direction != 0xFFF){
            Serial.println("Ang: " + String(direction));
            direction = standard_dir(target_dir, direction);

            if(abs(target_dir - direction) < error_angle){
                motor_left.stop();
                Serial.println("Stoped: " + String(rotate_timeout - time_out + millis()));
                return;
            }

            int rotate_speed = -auto_rotate_speed * rotate_pid.compute(target_dir, direction)/255;
            #ifdef DEBUG
                String message = String(rotate_speed) + '\t' + String(target_dir-direction);
                SerialBT.println(message);
            #endif
            motor_left.setSpeed(rotate_speed);
        }
    }
    motor_left.stop();
    motor_right.stop();
}


void reset_direction(HardwareSerial &serialPort = Serial){
    Serial.println("Reset direction");
    serialPort.println("AT+RST");
    delay(1000);
    serialPort.println("AT+RST");
    target_dir = 0;
}



void process_combo(int value){
    if(value == 0)  reset_direction(Serial2);
    if(value == 11) prepare_D30();
    if(value == 12) take_D30();
    if(value == 13) prepare_D40();
    if(value == 14) take_D40();

    if(value == 16) forward_command("OA0");
    if(value == 17) forward_command("O21");

    if(value == 20) auto_forward(1200);
    if(value == 29) auto_forward(4000);
    if(value == 30) auto_backward(1200);
    if(value == 31) auto_run_trajectory();
    if(value == 21) rote_CCW();
    if(value == 22) rote_CW();
}

#define ROTATE_PID
void update_k_PID(String command){
    float value = command.substring(1).toFloat();
    #ifdef FORWARD_PID
        if (command.startsWith("P")) {
            forward_pid.set_P(value);
        }
        if (command.startsWith("I")) {
            forward_pid.set_I(value);
        }
        if (command.startsWith("D")) {
            forward_pid.set_D(value);
        }
    #endif
    

    #ifdef ROTATE_PID
      if (command.startsWith("P")) {
          rotate_pid.set_P(value);
      }
      if (command.startsWith("I")) {
          rotate_pid.set_I(value);
      }
      if (command.startsWith("D")) {
          rotate_pid.set_D(value);
      }
    #endif
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

    if(prefix == 'E'){
        emergency_stop = true;
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

    if(prefix == 'k'){  // update_PID
        update_k_PID(command.substring(1));
        return;
    }

}

#endif