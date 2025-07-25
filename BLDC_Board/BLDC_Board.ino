// Controller: ESP32 LOLIN32 LITE
// Receive command from mainboard or Bluetooth
// Process to control BLDC motor / BLDC Servo (with encoder)
// Process PID to keep straight forward with signal from Hi229
// 2 BLDC channel with AB encoder and 2 BLDC channel with plush encoder
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;




#include "BLDC_motor.h"
#include <EncoderTimer.h>
#include <Encoder.h>


//           pwm, dir, brake, speed, dir, brake
BLDC_Motor motor_left( 16,  5, 17, 0, 1, 1);
BLDC_Motor motor_right(22, 23, 19, 0, 0, 1);

BLDC_Motor  slider_motor(27, 26, 14, 1, 0, 1);
Encoder     slider_encoder(34, 35);


int wheel_speed = 0;
void setup() {
    Serial.begin(115200);
    SerialBT.begin("BLDC"); // Set the Bluetooth device name
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
    show_encoder();
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
    if(ch == '0')   slider_motor.stop();
    if(ch == '*')   slider_motor.setSpeed(255);
    if(ch == '/')   slider_motor.setSpeed(-200);
    if(ch == '+')   slider_motor.setSpeed(100);
    if(ch == '-')   slider_motor.setSpeed(-80);
}


void process_vaccum(char ch){
    if(ch == '0')   forward_command("OA0");
    if(ch == '1')   forward_command("OA1");
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