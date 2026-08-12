// Controller: Arduino nano

// Receive command from controller via JDY-31 on Serial port
// Process and send command to other boards via UART

#include <Servo.h>

#define DEBUG


#define OPEN_ANGLE_SERVO 90
#define CLOSE_ANGLE_SERVO 0
#define SERVO_DELAY 200

const int servo_pins[4] = {14, 15, 16, 17};
const int drop_delay[4] = {0, 1000, 1500, 2000};

Servo servo[4];


void setup() {
    Serial.begin(9600);
    for(int i = 0; i < 4; i++)  servo[i].attach(servo_pins[i]);
    servo[0].write(CLOSE_ANGLE_SERVO);
    for(int i = 1; i < 4; i++)  servo[i].write(OPEN_ANGLE_SERVO);
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


void my_delay(int value){
    long time_out = millis() + value;
    while(millis() < time_out){
        signal_receriver();
    }
}

void drop_ball(int val){
    // Đóng ngăn chứa với số banh tương ứng
    servo[val].write(CLOSE_ANGLE_SERVO);
    my_delay(SERVO_DELAY);

    // Mở cửa để thả banh ra
    servo[0].write(OPEN_ANGLE_SERVO);
    my_delay(drop_delay[val]);

    // Đóng cửa và mở lại các ngăn chứa
    servo[0].write(CLOSE_ANGLE_SERVO);
    servo[val].write(OPEN_ANGLE_SERVO);
}

void process_command(String command){
    Serial.println(command);
    command.trim();
    char prefix = command.charAt(0);
    if(prefix == 'R'){
        int val = command.substring(1).toInt();
        drop_ball(val);
        return;
    }

    Serial.println("ERR: Invalid command");
}
