#ifndef CONFIG_H
#define CONFIG_H

// ==================== ENCODER PINS ====================
#define LEFT_ENCODER_PIN        13
#define RIGHT_ENCODER_PIN       18

// ==================== LINE SENSOR PINS (KY-038) ====================
// Tầng 1 (Chassis / Gầm xe): Dàn 4 mắt dò line
// Nhìn từ đuôi robot lên phía trước:
// L2: Ngoài cùng Trái, L1: Trong Trái, R1: Trong Phải, R2: Ngoài cùng Phải
#define LINE_PIN_L2             35
#define LINE_PIN_L1             34
#define LINE_PIN_R1             39
#define LINE_PIN_R2             36

// Cực tính cảm biến: Nền sáng = HIGH, Line đen = LOW
// => Khi mắt thấy line đen, ngõ ra = LOW => dùng ACTIVE_LOW
#define LINE_SENSOR_POLARITY    LineSensor::ACTIVE_LOW

// ==================== LINE TRACKING & STOP PARAMS ====================
#define LINE_SEARCH_SPEED       100     // Tốc độ di chuyển tìm line an toàn
#define LINE_SLOW_SPEED         60      // Tốc độ bò chậm để căn vuông góc
#define LINE_TIMEOUT_MS         5000    // Timeout tối đa mỗi chặng (chống deadlock)

// Góc chỉnh hướng khi L1/R1 chạm line dọc (đơn vị 0.1°, 10 = 1 độ)
#define DELTA_ANGLE             10

// Debounce thời gian giữa mỗi lần cập nhật target_dir (ms)
#define LINE_DIR_UPDATE_MS      2000

#endif
