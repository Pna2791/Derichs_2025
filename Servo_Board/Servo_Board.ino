// controller: Arduino nano
// Receive command from mainboard and process to control 4 servos on this board

#include <Servo.h>

#define DEBUG

#define n_servos  4
const int servo_pins[n_servos]  = {14, 15, 16, 17}; // Tương ứng với chân A0, A1, A2, A3 trên Nano
Servo servos[n_servos];

void setup() {
    for(int i=0; i<n_servos; i++){
        servos[i].attach(servo_pins[i]);
        servos[i].write(90); // Mặc định về góc 90 độ
    }

    Serial.begin(9600);
}

void signal_receriver(){
    static String command = "";
    if(Serial.available()){
        char ch = Serial.read();
        if(ch == '\n'){
            process_command(command);
            command = "";
        }else{
            command += ch;
        }
    }
}

// State machine cho tính năng thả bóng
int balls_to_drop = 0;
int current_sequence_step = -1; 
unsigned long next_action_time = 0;
const unsigned long drop_interval = 400; // 0.4s

void process_ball_drop() {
    if (balls_to_drop > 0) {
        if (millis() >= next_action_time) {
            if (current_sequence_step == -1) {
                current_sequence_step = 0;
            }
            
            // 4 servos, mỗi servo có 2 trạng thái: đập xuống (0) và dở lên (90)
            // Tổng cộng 8 bước (từ 0 đến 7)
            int servo_idx = current_sequence_step / 2;
            bool is_down = (current_sequence_step % 2 == 0);
            
            // Mặc định tất cả servo dở lên (90 độ) để giữ bóng
            for (int i = 0; i < n_servos; i++) {
                servos[i].write(90);
            }
            
            // Servo nào đến lượt thì đập xuống (0 độ)
            if (is_down) {
                servos[servo_idx].write(0);
            }
            
            next_action_time = millis() + drop_interval;
            current_sequence_step++;
            
            // Hoàn thành 1 chu kỳ thả 1 quả bóng (đủ 4 servo)
            if (current_sequence_step >= n_servos * 2) {
                balls_to_drop--;
                current_sequence_step = -1; // Reset để chuẩn bị thả quả tiếp theo
            }
        }
    }
}

void loop() {
    signal_receriver();
    process_ball_drop();
}

void process_command(String command){
    command.trim();
    if(command.length() == 0) return;

    // Kế thừa format của On_Off_Board: Ocx (ví dụ O01)
    // O là prefix, c là kênh (0-3), x là trạng thái (1/0)
    if (command.length() == 3 && command.charAt(0) == 'O') {
        char onoff = command.charAt(2);
        char channel = command.charAt(1);
        
        if ((onoff == '0' || onoff == '1') && channel >= '0' && channel <= '3') {
            int ch = channel - '0';
            int angle = (onoff == '1') ? 180 : 0; // 1 = 180 độ, 0 = 0 độ
            servos[ch].write(angle);

            #ifdef DEBUG
                Serial.print("OK: Servo ");
                Serial.print(ch);
                Serial.print(" moved to ");
                Serial.println(angle);
            #endif
            return;
        }
        
        // Cấu hình tất cả Servo cùng lúc bằng lệnh OA1 hoặc OA0
        if (channel == 'A'){
            int angle = (onoff == '1') ? 180 : 0;
            for(int i=0; i<n_servos; i++){
                servos[i].write(angle);
            }
            return;
        }
    }
    
    // Nút chức năng Combo trên app (Ví dụ C11, C12...)
    // Dùng để test đồng loạt cả 4 servo
    if (command.length() == 3 && command.charAt(0) == 'C') {
        for(int i=0; i<n_servos; i++) servos[i].write(180);
        delay(500);
        for(int i=0; i<n_servos; i++) servos[i].write(90);
        return;
    }

    // Tính năng thả bóng (Lệnh D)
    // Ví dụ: D1 (thả 1 quả), D2 (thả 2 quả), D3 (thả 3 quả)...
    if (command.charAt(0) == 'D') {
        int num_balls = command.substring(1).toInt();
        if (num_balls > 0) {
            #ifdef DEBUG
                Serial.print("Queueing ");
                Serial.print(num_balls);
                Serial.println(" ball(s) to drop...");
            #endif
            balls_to_drop += num_balls;
        }
        return;
    }

    Serial.println("ERR: Invalid or Unhandled command");
}
