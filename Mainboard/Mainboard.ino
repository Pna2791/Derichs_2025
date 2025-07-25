// Controller: Arduino nano

// Receive command from controller via JDY-31 on Serial port
// Process and send command to other boards via UART


#define DEBUG



void setup() {
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


void forward_command(String command){
    Serial.println(command);
}


void loop() {
    signal_receriver();
}


void stop_chassis(){
    forward_command("M0");
}


void process_command(String command){
    command = command.trim();

    char prefix = command.charAt(0);

    if(prefix == 'M'){  // Chassis
        if(command.length() == 3)
            forward_command(command);
        return;
    }

    Serial.println("ERR: Invalid command");
}
