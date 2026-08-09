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
#include "TaskQueue.h"


Encoder     left_encoder(13);
Encoder     right_encoder(18);

//           pwm, dir, brake, speed, dir, brake
BLDC_Motor motor_left( 16,  5, 17, 1, 0, 1);
BLDC_Motor motor_right(22, 23, 19, 1, 1, 1);

PIDController   forward_pid(10, 2, 1, -192, 192); // 315rpm speed 100-150
PIDController   rotate_pid(2, 0.0, 0.15, -255, 255); // 315rpm speed 100-150


TaskQueue forward_task_queue;
bool is_started       = false;
bool is_continue    = false;

int line_sensor_pins[4] = {35, 34, 36, 39};

bool check_line_sensor(int sensor_index){
    return analogRead(line_sensor_pins[sensor_index]) > 1000;
}

bool servo_enable = false;
bool emergency_stop = false;
bool calibrate_center_on = true;
int delta_angle_target = 0;

int target_dir = 0;
int wheel_speed = 0;

void processSerialCommand(String command);

void setup() {
    Serial.begin(115200);
    SerialBT.begin(ROBOT_NAME); // Set the Bluetooth device name
    Serial1.begin(9600, SERIAL_8N1, 0, 13); // RX, TX use for Second board
    Serial2.begin(115200, SERIAL_8N1, 2, 15); // RX, TX use for Hi229

    left_encoder.begin();
    right_encoder.begin();

    motor_left.stop();
    motor_right.stop();
    for(int i = 0; i < 4; i++){
        pinMode(line_sensor_pins[i], INPUT);
    }

    delay(1000);
    Serial.println("Started");
}


void forward_command(String command){
    Serial.println(command);
    Serial1.println(command);
}


void show_encoder(){
    long left_pos = left_encoder.getCount();
    long right_pos = right_encoder.getCount();
    String message = String(left_pos) + " \t" + String(right_pos);
    Serial.println(message);
}

void show_line_sensor(){
    String message = String(check_line_sensor(0)) + " " + String(check_line_sensor(1)) + " " + String(check_line_sensor(2)) + " " + String(check_line_sensor(3));
    Serial.println(message);
}


void update_servo(){
    static long next_update = millis();
    if(millis() > next_update){
        if(servo_enable){
            show_encoder();
        }
        show_line_sensor();
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


#define DEBUG

#define DELTA_ANGLE_TUNED 50
void calibrate_center(bool reset = false){
    static long next_update = millis();
    if(reset){
        delta_angle_target = 0;
        next_update = millis() + 500;
        return;
    }
    if(millis() > next_update){
        if(calibrate_center_on){
            if(check_line_sensor(0)){
                delta_angle_target += DELTA_ANGLE_TUNED;
                next_update = millis() + 500;
            }
            if(check_line_sensor(1)){
                delta_angle_target -= DELTA_ANGLE_TUNED;
                next_update = millis() + 500;
            }
        }
    }

}

// Calibrate with auto forward 4000mm (C29) the real distance should be 4150mm.
// If the distance larger than 4150mm, increase WHEEL_DIAMETER
// If the distance smaller than 4150mm, decrease WHEEL_DIAMETER
// new_dia = old_dia * distance_calibrate / 4150

// Hiệu chỉnh bằng cách chạy tự động 4000mm (lệnh C29) khoảng cách thực tế nên là 4150mm.
// Nếu khoảng cách đo được lớn hơn 4150mm, hãy tăng WHEEL_DIAMETER
// Nếu khoảng cách đo được nhỏ hơn 4150mm, hãy giảm WHEEL_DIAMETER
// Đường kính_mới = Đường kính_cũ * khoảng_cách_hiệu_chỉnh / 4150

#define WHEEL_DIAMETER 100
#define GEAR_RATIO 14   
#define STEPS_PER_REVOLUTION 6

// gear_ratio * steps_per_revolution / wheel_diameter / pi
const float step_per_mm = 1.0f * GEAR_RATIO * STEPS_PER_REVOLUTION / WHEEL_DIAMETER / 3.1416;

#define brake_distance      0       // Khoảng cách phanh khi dừng (hiện tại đang bỏ qua)
#define slowdown_distance   300     // Khoảng cách giảm tốc khi đến điểm dừng (Đây là điểm dừng khi không dò được, nếu không dò được thì quảng đường robot đã đi nhiều hơn slowdown_distance/2)
#define auto_forward_speed  120
// distance > 0: forward, distance < 0: backward (encoders count up only)
void auto_forward(int distance, int stop_sensor_index = 3){
    calibrate_center(true);
    
    int dir = (distance >= 0) ? 1 : -1;
    distance = abs(distance) + slowdown_distance/2;

    Serial.println("Auto forward: " + String(dir * distance));
    Serial.println("step_per_mm: " + String(step_per_mm));
    forward_pid.reset();
    float delta_plush = step_per_mm * (distance-brake_distance);
    long left_pos = left_encoder.getCount() + delta_plush;
    long right_pos = right_encoder.getCount() + delta_plush;
    Serial.println("Left target pos: " + String(left_pos));
    Serial.println("Right target pos: " + String(right_pos));

    int auto_speed = dir * auto_forward_speed;
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
            auto_speed = dir * auto_forward_speed * 0.4;
            is_normal_speed = false;
        }
        if(!is_normal_speed){
            if(check_line_sensor(stop_sensor_index)){
                break;
            }
        }
        
        my_loop();
        if(emergency_stop){
            Serial.println("Emergency stopped");
            motor_left.stop();
            motor_right.stop();
            return;
        }
        calibrate_center(false);

        int direction = get_direction(Serial2);
        if(direction != 0xFFF){
            Serial.println("Ang: " + String(direction));
            direction = standard_dir(target_dir+delta_angle_target, direction);

            float delta_value = forward_pid.compute(target_dir+delta_angle_target, direction)/255*dir;
            #ifdef DEBUG
                String message = String(delta_value*10) + '\t' + String(target_dir+delta_angle_target-direction);
                SerialBT.println(message);
            #endif

            motor_left.setSpeed(auto_speed * (1 - delta_value));
            motor_right.setSpeed(auto_speed * (1 + delta_value));
        }
    }

    motor_left.stop();
    motor_right.stop();
    Serial.println("Finish forward: " + String(dir * distance));
    Serial.println("Left current pos: " + String(left_encoder.getCount()));
    Serial.println("Right current pos: " + String(right_encoder.getCount()));
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
    delta_angle_target = 0;
}



const int abs_positions[7] = {0, 1000, 2000, 3260, 4520, 5520, 6520};
const int drop_delay[4] = {0, 1000, 1500, 2000};
int current_position_index = 3;

void process_task_queue(uint8_t val){
    int position = val / 10;
    int n_ball = val % 10;
    int distance = abs_positions[position] - abs_positions[current_position_index];
    if (distance != 0)
        auto_forward(distance);
    
    forward_command("R" + String(n_ball));
    my_delay(drop_delay[n_ball]);
    current_position_index = position;
}

void simple_strategy(){
    forward_task_queue.enqueue(31); // Drop 1 ball at position 3
    forward_task_queue.enqueue(41); // Drop 1 ball at position 4
    forward_task_queue.enqueue(51); // Drop 1 ball at position 5
    forward_task_queue.enqueue(61); // Drop 1 ball at position 6

    calibrate_center_on = false;
    auto_forward(3600);

    // Wait for continue signal
    while(!is_continue){
        my_loop();
        if(emergency_stop)
            return;
    }
    rote_CW();
    
    auto_forward(1850, 2);
    rote_CCW();
    auto_forward(-1700);
    current_position_index = 3;
    calibrate_center_on = true;
    

    // Start dropping
    while(true){
        // check task in queue
        uint8_t val;
        if (forward_task_queue.dequeue(val)) {
            process_task_queue(val);
        }

        my_loop();
        if(emergency_stop){
            is_started = false;
            is_continue = false;
            forward_task_queue.clear();
            return;
        }
    }
}


void process_combo(int value){
    if(value == 0)  reset_direction(Serial2);
    if(value == 16) forward_command("OA0");
    if(value == 17) forward_command("O21");

    if(value == 20) auto_forward(1200);
    if(value == 25) auto_forward(-1200);
    if(value == 29) auto_forward(4000);
    if(value == 21) rote_CCW();
    if(value == 22) rote_CW();

    if(value == 30) simple_strategy();
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


void process_task_queue(int val){
    if(val == 9){
        if(is_started){
            is_continue = true;
        }else{
            is_started = true;
            is_continue = false;
            reset_direction();
            simple_strategy();
        }
        return;
    }
    if(val%10 < 4 && val/10 < 7 && val/10 > 0){
        forward_task_queue.enqueue(val);
        return;
    }
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

    if(prefix == 'E'){
        emergency_stop = true;
        return;
    }
    if(prefix == 'F'){
        int value = command.substring(1).toInt();
    }

    if(prefix == 'C'){  // Chassis direction
        int value = command.substring(1).toInt();
        process_combo(value);
        return;
    }

    if(prefix == 'O'){  // ON/OFF
        if(command.length() == 3)
            forward_command(command);
        return;
    }
    
    if(prefix == 'k'){  // update_PID
        update_k_PID(command.substring(1));
        return;
    }

}

#endif