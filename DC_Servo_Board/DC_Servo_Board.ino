// Controller: Arduino Nano (DC_Servo_Board)
// Quản lý 3 RC Servo (A1, A2, A3) và Cảm biến hồng ngoại đếm bóng HW-488 (Pin 3)

#include <Servo.h>

Servo ser1, ser2, ser3;

#define SENS_PIN 3
#define SER1_MO 180
#define SER2_MO 180
#define SER3_MO 180
#define SER1_DONG 0
#define SER2_DONG 0
#define SER3_DONG 0

// Trạng thái State Machine
#define COMBO_IDLE           0
#define COMBO_SER2_TIMER     1
#define COMBO_DROPPING_BALLS 2
#define COMBO_FINAL_STEP     3

int mode_auto = 1;
int trangThaiCombo = COMBO_IDLE;
unsigned long thoiGianLuu = 0;

int soBongDaDem = 0;
int sobongcantha = 3;

bool trangThaiSensCu = HIGH;
bool checkline = false;

void docserial();
void xuLyCombo();
void xuLyDemBong();
void xuLyTimerServo();

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(10);
    pinMode(SENS_PIN, INPUT_PULLUP);

    ser1.attach(A1);
    ser2.attach(A2);
    ser3.attach(A3);

    // SER 1 luôn mở khi khởi động, SER 2 & SER 3 đóng
    ser1.write(SER1_MO);
    ser2.write(SER2_DONG);
    ser3.write(SER3_DONG);

    Serial.println("Ready");
}

void loop() {
    docserial();
    xuLyCombo();
}

void docserial() {
    if (Serial.available() <= 0) return;
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "S2_2S" || cmd == "SER2_2S") {
        ser2.write(SER2_MO);
        thoiGianLuu = millis() + 2000;
        trangThaiCombo = COMBO_SER2_TIMER;
    } else if (cmd == "S2_1S" || cmd == "SER2_1S") {
        ser2.write(SER2_MO);
        thoiGianLuu = millis() + 1000;
        trangThaiCombo = COMBO_SER2_TIMER;
    } else if (cmd == "S2_3S" || cmd == "SER2_3S") {
        ser2.write(SER2_MO);
        thoiGianLuu = millis() + 3000;
        trangThaiCombo = COMBO_FINAL_STEP;
    } else if (cmd == "B3" || cmd == "DROP3") {
        sobongcantha = 3; soBongDaDem = 0;
        ser3.write(SER3_MO);
        Serial.println("D1");
        trangThaiCombo = COMBO_DROPPING_BALLS;
    } else if (cmd == "B1" || cmd == "DROP1") {
        sobongcantha = 1; soBongDaDem = 0;
        ser3.write(SER3_MO);
        Serial.println("D1");
        trangThaiCombo = COMBO_DROPPING_BALLS;
    } else if (cmd == "B2" || cmd == "DROP2") {
        sobongcantha = 2; soBongDaDem = 0;
        ser3.write(SER3_MO);
        Serial.println("D1");
        trangThaiCombo = COMBO_DROPPING_BALLS;
    } else if (cmd == "E") {
        ser1.write(SER1_DONG); ser2.write(SER2_DONG); ser3.write(SER3_DONG);
        Serial.println("D0");
        trangThaiCombo = COMBO_IDLE;
    }
}

void xuLyDemBong() {
    bool docSens = digitalRead(SENS_PIN);
    static unsigned long debounceTime = 0;

    // Phát hiện cạnh rơi HIGH -> LOW khi bóng đi qua cảm biến
    if (trangThaiSensCu == HIGH && docSens == LOW && (millis() - debounceTime > 40)) {
        soBongDaDem++;
        debounceTime = millis();
        Serial.print("BALL: ");
        Serial.println(soBongDaDem);
    }
    trangThaiSensCu = docSens;

    if (soBongDaDem >= sobongcantha) {
        ser3.write(SER3_DONG);
        Serial.println("D0");
        Serial.println("DONE");
        trangThaiCombo = COMBO_IDLE;
    }
}

void xuLyTimerServo() {
    if (millis() < thoiGianLuu) return;

    if (trangThaiCombo == COMBO_SER2_TIMER) {
        ser2.write(SER2_DONG);
        Serial.println("SER2_DONE");
        trangThaiCombo = COMBO_IDLE;
    } else if (trangThaiCombo == COMBO_FINAL_STEP) {
        ser2.write(SER2_DONG);
        ser1.write(SER1_DONG);
        Serial.println("ALL_DONE");
        trangThaiCombo = COMBO_IDLE;
    }
}

void xuLyCombo() {
    if (trangThaiCombo == COMBO_DROPPING_BALLS) {
        xuLyDemBong();
    } else if (trangThaiCombo == COMBO_SER2_TIMER || trangThaiCombo == COMBO_FINAL_STEP) {
        xuLyTimerServo();
    }
}