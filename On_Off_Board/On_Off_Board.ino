// controller: Arduino nano
// Receive command from mainboard and process to turn on/off 9 ports on this board

// On Off command start with O, format is "Oxc" - O is prefix, x is On/Off (1, 0), c is channel from 0-8


#define DEBUG


#define n_channels  9
const int channel_pins[n_channels]  = {3, 4, 5, 6, 7, 8, 9, 10, 11};

#define n_feedback  4
const int feedback_input_pins[n_feedback]   = {A4, A5, A6, A7};
const int feedback_output_pins[n_feedback]  = {A0, A1, A2, A3};
const int feedback_thresholds[n_feedback]   = {1024, 1024, 1024, 1024};



void setup() {
    for(int i=0; i<n_channels; i++)
        pinMode(channel_pins[i], OUTPUT);
    
    for(int i=0; i<n_feedback; i++){
        pinMode(feedback_input_pins[i], INPUT);
        pinMode(feedback_output_pins[i], OUTPUT);
    }

    Serial.begin(9600);
}


void process_current(){
    static int count = 0;
    static int feedback_values[n_feedback]  = {0, 0, 0, 0};

    // Just process with 1 sample / ms
    static long next_update = millis();
    if(millis() < next_update)  return;
    next_update += 1;

    for(int i=0; i<n_feedback; i++){
        feedback_values[i] += analogRead(feedback_input_pins[i]);
        feedback_values[i] /= 2;
    }


    count++;
    // Check status with 100Hz
    if(count >= 10){
        for(int i=0; i<n_feedback; i++){
            bool status = feedback_values[i] > feedback_thresholds[i];
            digitalWrite(feedback_output_pins[i], status);
            Serial.print(feedback_values[i]);
            Serial.print('\t');
        }
        Serial.println();

        count = 0;
    }
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
    // process_current();
}


void process_command(String command){
    command.trim();

    // Expecting format: Oxc (e.g., O10) where x=1/0, c=0-8
    if (command.length() == 3 && command.charAt(0) == 'O') {
        char onoff = command.charAt(2);
        char channel = command.charAt(1);
        if ((onoff == '0' || onoff == '1') && channel >= '0' && channel <= '8') {
            int ch = channel - '0';
            int state = (onoff == '1') ? HIGH : LOW;
            digitalWrite(channel_pins[ch], state);

            #ifdef DEBUG
                Serial.print("OK: Channel ");
                Serial.print(ch);
                Serial.print(" set to ");
                Serial.println(state == HIGH ? "ON" : "OFF");
            #endif
            return;
        }
        if (channel == 'A'){
            int state = (onoff == '1') ? HIGH : LOW;
            for(int i=0; i<n_channels; i++){
                digitalWrite(channel_pins[i], state);
            }
            return;
        }
    }
    Serial.println("ERR: Invalid command");
}
