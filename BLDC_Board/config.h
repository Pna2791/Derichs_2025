#ifndef CONFIG_H
#define CONFIG_H

// ==================== ENCODER PINS ====================
#define LEFT_ENCODER_PIN        13
#define RIGHT_ENCODER_PIN       18

// ==================== LINE SENSOR PINS (KY-038) ====================
// Tầng 1 (Chassis / Gầm xe): Dàn 4 mắt dò line
// L2: Ngoài cùng Trái, L1: Trong Trái, R1: Trong Phải, R2: Ngoài cùng Phải
#define LINE_PIN_L2             4
#define LINE_PIN_L1             27
#define LINE_PIN_R1             32
#define LINE_PIN_R2             33

// Tầng Servo / Phụ trợ (Optional detection pins)
#define LINE_PIN_SERVO_1        34
#define LINE_PIN_SERVO_2        35

// Cực tính cảm biến: KY-038 xuất mức HIGH khi vào line đen (nền sáng = LOW)
#define LINE_SENSOR_POLARITY    LineSensor::ACTIVE_HIGH

// ==================== LINE TRACKING & STOP PARAMS ====================
#define LINE_SEARCH_SPEED       100     // Tốc độ di chuyển tìm line an toàn
#define LINE_SLOW_SPEED         60      // Tốc độ bò chậm để căn vuông góc
#define LINE_TIMEOUT_MS         5000    // Timeout tối đa nếu trượt line (chống deadlock)

#endif