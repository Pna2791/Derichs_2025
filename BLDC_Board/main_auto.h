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
#include "LineSensor.h"


Encoder     left_encoder(LEFT_ENCODER_PIN);
Encoder     right_encoder(RIGHT_ENCODER_PIN);

// Khởi tạo dàn 4 mắt cảm biến KY-038 tầng 1
LineSensor  line_sensor(LINE_PIN_L2, LINE_PIN_L1, LINE_PIN_R1, LINE_PIN_R2, LINE_SENSOR_POLARITY);

//           pwm, dir, brake, speed, dir, brake
BLDC_Motor motor_left( 16,  5, 17, 1, 0, 1);
BLDC_Motor motor_right(22, 23, 19, 1, 1, 1);

PIDController   forward_pid(10, 2, 1, -192, 192); // 315rpm speed 100-150
PIDController   rotate_pid(2, 0.0, 0.15, -255, 255); // 315rpm speed 100-150
PIDController   line_pid(120, 0.0, 10, -180, 180);  // PID bám line


bool servo_enable = false;
bool emergency_stop = false;

int target_dir = 0;
int wheel_speed = 0;

void processSerialCommand(String command);

void setup() {
    Serial.begin(115200);
    SerialBT.begin(ROBOT_NAME); // Set the Bluetooth device name
    Serial2.begin(115200, SERIAL_8N1, 2, 15); // RX, TX use for Hi229

    left_encoder.begin();
    right_encoder.begin();
    line_sensor.begin();

    motor_left.stop();
    motor_right.stop();

    delay(1000);
    Serial.println("Started with Line Sensor Support");
}



void forward_command(String command){
    Serial.println(command);
}


void show_encoder(){
    static long next_update = millis();

    if(millis() > next_update){
        long left_pos = left_encoder.getCount();
        long right_pos = right_encoder.getCount();
        String message = String(left_pos) + " \t" + String(right_pos);
        Serial.println(message);

        next_update += 1000;
    }
}


void update_servo(){
    static long next_update = millis();
    if(millis() > next_update){
        if(servo_enable){
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

#define WHEEL_DIAMETER 100
#define GEAR_RATIO 14   
#define STEPS_PER_REVOLUTION 6

// gear_ratio * steps_per_revolution / wheel_diameter / pi
const float step_per_mm = 1.0f * GEAR_RATIO * STEPS_PER_REVOLUTION / WHEEL_DIAMETER / 3.1416;

#define brake_distance      10
#define slowdown_distance   250
#define auto_forward_speed  120
// distance > 0: forward, distance < 0: backward (encoders count up only)
void auto_forward(int distance){
    int dir = (distance >= 0) ? 1 : -1;
    distance = abs(distance);

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

            float delta_value = forward_pid.compute(target_dir, direction)/255*dir;
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
}

void simple_strategy(){
    auto_forward(3600);
    rote_CW();
    my_delay(5000);

    auto_forward(1200);
    rote_CCW();
    auto_forward(-1260);
    my_delay(1000);
    auto_forward(1260*2);
    my_delay(1000);
    auto_forward(1000);
    my_delay(1000);
    auto_forward(1000);
}


// ==================== LINE SENSOR NAVIGATION FUNCTIONS ====================

/**
 * @brief Chạy tiến kết hợp:
 *        - 2 mắt giữa L1/R1 kẹp vạch để tự động căn tâm (Line Centering).
 *        - IMU Hi229 giữ hướng thẳng khi line nằm lọt giữa L1 và R1.
 *        - 2 mắt ngoài L2/R2 đếm số vạch ngang / nút giao trên sa bàn để dừng chuẩn 100%.
 * @param target_lines Số vạch ngang cần đi qua để dừng xe
 * @param speed Tốc độ di chuyển
 * @param timeout_ms Thời gian tối đa chống kẹt/tuột line
 * @return true nếu đếm đủ số vạch và dừng thành công, false nếu timeout
 */
bool auto_forward_by_lines(int target_lines, int speed = LINE_SEARCH_SPEED, uint32_t timeout_ms = LINE_TIMEOUT_MS) {
    Serial.println("Auto forward by lines: Target " + String(target_lines) + " lines");
    forward_pid.reset();
    line_pid.reset();
    line_sensor.resetLineCounterState();

    int lines_passed = 0;
    int auto_speed = speed;
    motor_left.setSpeed(auto_speed);
    motor_right.setSpeed(auto_speed);

    long time_out = millis() + timeout_ms;

    while (millis() < time_out) {
        my_loop();
        if (emergency_stop) {
            Serial.println("Emergency stopped in line count");
            motor_left.stop();
            motor_right.stop();
            return false;
        }

        // 1. Kiểm tra phát hiện vạch ngang mới qua 2 mắt ngoài L2 / R2
        if (line_sensor.checkNewCrossLine()) {
            lines_passed++;
            Serial.println(">>> PASSED LINE: " + String(lines_passed) + " / " + String(target_lines) + 
                           " (Pos L=" + String(left_encoder.getCount()) + " R=" + String(right_encoder.getCount()) + ")");
            #ifdef DEBUG
                SerialBT.println("Line: " + String(lines_passed) + "/" + String(target_lines));
            #endif

            // Đã đạt đủ số vạch ngang mục tiêu -> Phanh khẩn cấp & căn góc
            if (lines_passed >= target_lines) {
                motor_left.stop();
                motor_right.stop();
                Serial.println(">>> REACHED TARGET NODE! Stopped perfectly.");
                delay(50);
                auto_align_to_line(LINE_SLOW_SPEED, 1500); // Tự động căn vuông góc 90 độ với vạch
                return true;
            }
        }

        // 2. Điều hướng: Kết hợp bám tâm (L1, R1 kẹp vạch) và IMU Hi229
        float steer_err = line_sensor.getSteeringError(); // L1 chạm -> -1.0 (lái trái), R1 chạm -> +1.0 (lái phải), giữa -> 0.0
        float delta_value = 0.0f;

        if (steer_err != 0.0f) {
            // Khi bị lệch khỏi khe giữa -> Bẻ lái sửa tâm theo cảm biến
            delta_value = line_pid.compute(0, steer_err * 100.0f) / 255.0f;
        } else {
            // Khi đang ở ngay chính giữa khe -> Dùng IMU Hi229 giữ hướng thẳng hoàn hảo
            int direction = get_direction(Serial2);
            if (direction != 0xFFF) {
                direction = standard_dir(target_dir, direction);
                delta_value = forward_pid.compute(target_dir, direction) / 255.0f;
            }
        }

        motor_left.setSpeed(auto_speed * (1.0f - delta_value));
        motor_right.setSpeed(auto_speed * (1.0f + delta_value));
    }

    motor_left.stop();
    motor_right.stop();
    Serial.println("Line count timeout! Passed " + String(lines_passed) + "/" + String(target_lines));
    return false;
}

/**
 * @brief Tiến thẳng kết hợp giữ góc IMU và dừng ngay khi phát hiện vạch line đầu tiên
 */
bool auto_forward_until_line(int max_distance, int speed = LINE_SEARCH_SPEED, uint32_t timeout_ms = LINE_TIMEOUT_MS) {
    return auto_forward_by_lines(1, speed, timeout_ms);
}

/**
 * @brief Tự động căn vuông góc 90 độ với vạch ngang (Squaring to line)
 *        Bánh bên nào chưa chạm vạch ngoài thì nhích tiếp, bên nào chạm rồi thì dừng lại.
 */
bool auto_align_to_line(int align_speed = LINE_SLOW_SPEED, uint32_t timeout_ms = 3000) {
    Serial.println("Aligning to line...");
    long time_out = millis() + timeout_ms;

    while (millis() < time_out) {
        my_loop();
        if (emergency_stop) {
            motor_left.stop();
            motor_right.stop();
            return false;
        }

        bool left_on = line_sensor.isLeftTriggered();   // Mắt ngoài L2
        bool right_on = line_sensor.isRightTriggered(); // Mắt ngoài R2

        // Cả 2 bên đều đã chạm vạch line -> Đã vuông góc 90 độ hoàn hảo
        if (left_on && right_on) {
            motor_left.stop();
            motor_right.stop();
            Serial.println("Line alignment complete (90 deg calibrated)!");
            return true;
        }

        // Bên trái đã chạm line -> dừng bánh trái, nhích bánh phải
        if (left_on && !right_on) {
            motor_left.stop();
            motor_right.setSpeed(align_speed);
        }
        // Bên phải đã chạm line -> dừng bánh phải, nhích bánh trái
        else if (!left_on && right_on) {
            motor_right.stop();
            motor_left.setSpeed(align_speed);
        }
        // Cả 2 chưa chạm -> cùng nhích chậm về phía trước
        else {
            motor_left.setSpeed(align_speed);
            motor_right.setSpeed(align_speed);
        }
    }

    motor_left.stop();
    motor_right.stop();
    Serial.println("Align timeout");
    return false;
}

/**
 * @brief Chạy bám theo đường line bằng thuật toán PID
 * @param distance_mm Quãng đường bám line (mm)
 * @param follow_speed Tốc độ di chuyển
 * @param timeout_ms Thời gian tối đa
 */
bool auto_follow_line(int distance_mm, int follow_speed = LINE_SEARCH_SPEED, uint32_t timeout_ms = LINE_TIMEOUT_MS) {
    Serial.println("Auto follow line: " + String(distance_mm) + " mm");
    line_pid.reset();

    float delta_plush = step_per_mm * distance_mm;
    long left_pos = left_encoder.getCount() + delta_plush;
    long right_pos = right_encoder.getCount() + delta_plush;

    long time_out = millis() + timeout_ms;

    while (millis() < time_out) {
        my_loop();
        if (emergency_stop) {
            motor_left.stop();
            motor_right.stop();
            return false;
        }

        // Đạt đủ quãng đường yêu cầu
        if (left_encoder.getCount() >= left_pos || right_encoder.getCount() >= right_pos) {
            break;
        }

        // Sai số căn tâm: L1 chạm -> -1.0 (lái trái), R1 chạm -> +1.0 (lái phải)
        float steer_error = line_sensor.getSteeringError();
        float delta_val = line_pid.compute(0, steer_error * 100.0f) / 255.0f;

        motor_left.setSpeed(follow_speed * (1.0f - delta_val));
        motor_right.setSpeed(follow_speed * (1.0f + delta_val));
    }

    motor_left.stop();
    motor_right.stop();
    Serial.println("Finish follow line");
    return true;
}

/**
 * @brief Chiến thuật Sa bàn chuẩn xác tuyệt đối: Đếm vạch ngang thay vì chạy mù khoảng cách
 *        Ánh xạ trực tiếp từ kích thước sa bàn 800x800cm và các mốc trong simple_strategy()
 */
void line_assisted_strategy() {
    Serial.println("Starting Line Assisted Strategy (Field 800x800cm)");

    // 1. Chặng 1: Xuất phát chạy qua 3 vạch ngang (thay cho auto_forward(3600))
    auto_forward_by_lines(3, LINE_SEARCH_SPEED, 8000);
    my_delay(500);

    // 2. Chặng 2: Xoay phải 90 độ, chạy qua 1 vạch ngang (thay cho auto_forward(1200))
    rote_CW();
    my_delay(500);
    auto_forward_by_lines(1, LINE_SEARCH_SPEED, 4000);
    my_delay(5000);

    // 3. Chặng 3: Xoay trái 90 độ, lùi 1 khoảng 1260mm
    rote_CCW();
    my_delay(500);
    auto_forward(-1260);
    my_delay(1000);

    // 4. Chặng 4: Tiến qua 2 vạch ngang (thay cho auto_forward(1260*2 = 2520mm))
    auto_forward_by_lines(2, LINE_SEARCH_SPEED, 6000);
    my_delay(1000);

    // 5. Chặng 5: Tiến qua 1 vạch ngang (thay cho auto_forward(1000))
    auto_forward_by_lines(1, LINE_SEARCH_SPEED, 4000);
    my_delay(1000);

    // 6. Chặng 6: Tiến qua 1 vạch ngang tiếp theo (thay cho auto_forward(1000))
    auto_forward_by_lines(1, LINE_SEARCH_SPEED, 4000);
    my_delay(1000);

    Serial.println("Line Strategy Complete!");
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

    // Các kịch bản mở rộng với cảm biến Line (Bảo toàn 100% mã cũ)
    if(value == 31) auto_forward_until_line(4000);       // Tiến tìm vạch ngang đầu tiên và dừng
    if(value == 32) auto_align_to_line();                // Tự động căn vuông góc với vạch line
    if(value == 33) auto_forward_by_lines(2);            // Chạy qua đúng 2 vạch ngang rồi dừng
    if(value == 34) line_assisted_strategy();           // Toàn bộ chiến thuật sa bàn đếm vạch chuẩn xác
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

    if(prefix == 'E'){
        emergency_stop = true;
        return;
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

    if(prefix == 'L'){  // Line Sensor Actions & Combos
        String sub = command.substring(1);
        sub.trim();

        // 1. Lệnh 'L' hoặc 'L0': In trạng thái cảm biến phục vụ căn chỉnh tại chỗ
        if(sub.length() == 0 || sub == "0"){
            uint8_t raw = line_sensor.readRawBits();
            uint8_t filtered = line_sensor.readFilteredBits();
            float err = line_sensor.getSteeringError();
            String msg = "Line Status -> Raw: [L2=" + String((raw>>3)&1) + " L1=" + String((raw>>2)&1) + 
                         " R1=" + String((raw>>1)&1) + " R2=" + String(raw&1) + 
                         "] | Filtered: 0b" + String(filtered, BIN) + 
                         " | Steering Err: " + String(err) + 
                         " | CrossLine: " + String(line_sensor.isCrossLine() ? "YES" : "NO");
            Serial.println(msg);
            SerialBT.println(msg);
            return;
        }

        int val = sub.toInt();

        // 2. L1 hoặc L31: Tiến tìm vạch ngang đầu tiên và dừng
        if(val == 1 || val == 31){
            auto_forward_until_line(4000);
            return;
        }

        // 3. L2 hoặc L32: Tự động căn vuông góc 90 độ với vạch ngang
        if(val == 2 || val == 32){
            auto_align_to_line();
            return;
        }

        // 4. L3 hoặc L33: Chạy qua đúng 2 vạch ngang rồi dừng
        if(val == 3 || val == 33){
            auto_forward_by_lines(2);
            return;
        }

        // 5. L4 hoặc L34: Chạy toàn bộ chiến thuật sa bàn 800x800cm
        if(val == 4 || val == 34){
            line_assisted_strategy();
            return;
        }

        // 6. L{N}: Chạy qua đúng N vạch ngang bất kỳ (ví dụ L5 -> chạy qua 5 vạch)
        if(val > 0){
            auto_forward_by_lines(val);
            return;
        }
    }


}

#endif