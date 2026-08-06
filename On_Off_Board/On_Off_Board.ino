// Controller: Arduino Nano (On_Off_Board)
// Quản lý 9 cổng công suất - Giao tiếp UART 115200
// Hỗ trợ song song: Bộ lệnh nhanh D1/D0/E và ma trận lệnh chuẩn O{Channel}{State}

#define n_channels 9
const int channel_pins[n_channels] = {3, 4, 5, 6, 7, 8, 9, 10, 11};

// Mapping 3 động cơ DC chính
#define DC1_PIN 3  // Channel 0 (Pin 3)
#define DC2_PIN 4  // Channel 1 (Pin 4)
#define DC3_PIN 5  // Channel 2 (Pin 5)

void process_command(String command);

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(10);

    for (int i = 0; i < n_channels; i++) {
        pinMode(channel_pins[i], OUTPUT);
    }

    // Động cơ DC 1 & DC 2 luôn chạy khi robot khởi động
    digitalWrite(DC1_PIN, HIGH);
    digitalWrite(DC2_PIN, HIGH);
    digitalWrite(DC3_PIN, LOW);
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        process_command(command);
    }
}

void process_command(String command) {
    command.trim();
    if (command.length() == 0) return;

    // 1. Bộ lệnh nhanh đồng bộ DC 3 và Dừng khẩn cấp
    if (command == "D1") {
        digitalWrite(DC3_PIN, HIGH);
        return;
    }
    if (command == "D0") {
        digitalWrite(DC3_PIN, LOW);
        return;
    }
    if (command == "E" || command == "OA0") {
        for (int i = 0; i < n_channels; i++) digitalWrite(channel_pins[i], LOW);
        return;
    }
    if (command == "OA1") {
        digitalWrite(DC1_PIN, HIGH);
        digitalWrite(DC2_PIN, HIGH);
        digitalWrite(DC3_PIN, LOW);
        return;
    }

    // 2. Bảo toàn ma trận giao thức chuẩn: O{Channel}{State} (O11, O20, OA0...)
    if (command.length() == 3 && command.charAt(0) == 'O') {
        char channel = command.charAt(1);
        char onoff   = command.charAt(2);
        if ((onoff == '0' || onoff == '1') && channel >= '0' && channel <= '8') {
            int ch = channel - '0';
            digitalWrite(channel_pins[ch], (onoff == '1') ? HIGH : LOW);
        }
    }
}
