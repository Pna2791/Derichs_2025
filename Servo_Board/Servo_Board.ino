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

void loop() {
    signal_receriver();
}

void drop_balls(int count) {
    for (int i = 0; i < count; i++) {
        // Giả sử Servo 0 (chân 14) là servo dùng để chặn/nhả bóng
        servos[0].write(0);   // Mở cửa chặn để 1 quả bóng rơi xuống
        delay(400);           // Đợi bóng rớt (thời gian này anh có thể tinh chỉnh lại)
        servos[0].write(90);  // Đóng cửa chặn lại
        delay(400);           // Đợi quả bóng tiếp theo lăn vào vị trí
    }
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
                Serial.print("Dropping ");
                Serial.print(num_balls);
                Serial.println(" ball(s)...");
            #endif
            drop_balls(num_balls);
        }
        return;
    }

    Serial.println("ERR: Invalid or Unhandled command");
}
