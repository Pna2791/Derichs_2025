#ifndef LINE_SENSOR_H
#define LINE_SENSOR_H

#include <Arduino.h>

/**
 * @brief Lớp quản lý dàn cảm biến Line 4 mắt (KY-038 / Cảm biến hồng ngoại)
 *        Hỗ trợ lọc nhiễu Debounce, phát hiện vạch ngang (Intersection)
 *        và tính toán sai số lệch tâm cho thuật toán PID dò line.
 */
class LineSensor {
public:
    enum SensorPolarity {
        ACTIVE_HIGH = 1, // Pin ra HIGH khi chạm line, LOW trên nền sáng
        ACTIVE_LOW = 0   // Pin ra LOW khi chạm line, HIGH trên nền sáng (dùng cho KY-038 đảo)
    };

    /**
     * @param p_left_outer   Chân cảm biến ngoài cùng bên trái (L2)
     * @param p_left_inner   Chân cảm biến trong bên trái (L1)
     * @param p_right_inner  Chân cảm biến trong bên phải (R1)
     * @param p_right_outer  Chân cảm biến ngoài cùng bên phải (R2)
     * @param polarity       Mức logic khi chạm vạch (mặc định ACTIVE_HIGH cho KY-038)
     */
    LineSensor(int p_left_outer, int p_left_inner, int p_right_inner, int p_right_outer, SensorPolarity polarity = ACTIVE_HIGH)
        : pin_l2(p_left_outer), pin_l1(p_left_inner), pin_r1(p_right_inner), pin_r2(p_right_outer),
          sensor_polarity(polarity), last_error(0.0f), has_detected(false) {}

    void begin() {
        if (pin_l2 >= 0) pinMode(pin_l2, INPUT);
        if (pin_l1 >= 0) pinMode(pin_l1, INPUT);
        if (pin_r1 >= 0) pinMode(pin_r1, INPUT);
        if (pin_r2 >= 0) pinMode(pin_r2, INPUT);
    }

    /**
     * @brief Đọc trạng thái số nguyên 4-bit của dàn cảm biến [L2, L1, R1, R2]
     * @return Byte 4-bit (bit 3: L2, bit 2: L1, bit 1: R1, bit 0: R2)
     */
    uint8_t readRawBits() {
        uint8_t bits = 0;
        bool raw_l2 = (pin_l2 >= 0) ? (digitalRead(pin_l2) == (int)sensor_polarity) : false;
        bool raw_l1 = (pin_l1 >= 0) ? (digitalRead(pin_l1) == (int)sensor_polarity) : false;
        bool raw_r1 = (pin_r1 >= 0) ? (digitalRead(pin_r1) == (int)sensor_polarity) : false;
        bool raw_r2 = (pin_r2 >= 0) ? (digitalRead(pin_r2) == (int)sensor_polarity) : false;

        if (raw_l2) bits |= (1 << 3);
        if (raw_l1) bits |= (1 << 2);
        if (raw_r1) bits |= (1 << 1);
        if (raw_r2) bits |= (1 << 0);

        return bits;
    }

    /**
     * @brief Bộ lọc Debounce quang học loại bỏ xung nhiễu ánh sáng sân đấu
     * @param debounce_samples Số mẫu liên tiếp cần đồng nhất
     * @return Bitmask 4-bit đã qua lọc: bit 3: L2, bit 2: L1, bit 1: R1, bit 0: R2
     */
    uint8_t readFilteredBits(uint8_t debounce_samples = 3) {
        uint8_t last_val = readRawBits();
        for (uint8_t i = 1; i < debounce_samples; i++) {
            delayMicroseconds(50);
            uint8_t current_val = readRawBits();
            if (current_val != last_val) {
                return filtered_bits; // Nếu bất ổn định, giữ trạng thái tin cậy trước đó
            }
        }
        filtered_bits = last_val;
        return filtered_bits;
    }

    bool isL2() { return (readFilteredBits() & (1 << 3)) != 0; } // Mắt ngoài Trái (Đếm vạch)
    bool isL1() { return (readFilteredBits() & (1 << 2)) != 0; } // Mắt trong Trái (Căn tâm)
    bool isR1() { return (readFilteredBits() & (1 << 1)) != 0; } // Mắt trong Phải (Căn tâm)
    bool isR2() { return (readFilteredBits() & (1 << 0)) != 0; } // Mắt ngoài Phải (Đếm vạch)

    /**
     * @brief Kiểm tra phát hiện vạch ngang / Nút giao (Intersection)
     *        Sử dụng 2 mắt ngoài cùng L2 và R2
     */
    bool isCrossLine() {
        uint8_t bits = readFilteredBits();
        // Khi mắt ngoài L2 hoặc R2 (hoặc cả 2) chạm vào vạch ngang
        return ((bits & (1 << 3)) != 0) || ((bits & (1 << 0)) != 0);
    }

    /**
     * @brief Kiểm tra cả 2 mắt ngoài cùng chạm vạch ngang (Vuông góc chuẩn)
     */
    bool isFullCrossLine() {
        uint8_t bits = readFilteredBits();
        return ((bits & (1 << 3)) != 0) && ((bits & (1 << 0)) != 0);
    }

    /**
     * @brief Kiểm tra phía bên trái chạm line ngoài
     */
    bool isLeftTriggered() {
        return isL2();
    }

    /**
     * @brief Kiểm tra phía bên phải chạm line ngoài
     */
    bool isRightTriggered() {
        return isR2();
    }

    /**
     * @brief Tính sai số căn tâm (Centering Error) cho 2 mắt giữa L1 và R1:
     *        - Đường line dọc nằm lọt ở KHE GIỮA L1 và R1.
     *        - Khi robot đi thẳng chuẩn: L1=LOW (0), R1=LOW (0) -> Error = 0.0f
     *        - Khi robot dạt sang PHẢI: vạch line chạm L1 (L1=HIGH, R1=LOW) -> Error = -1.0f (cần bẻ Lái Trái)
     *        - Khi robot dạt sang TRÁI: vạch line chạm R1 (L1=LOW, R1=HIGH) -> Error = +1.0f (cần bẻ Lái Phải)
     *        - Khi đi qua vạch ngang (L1=HIGH, R1=HIGH) -> Giữ thẳng Error = 0.0f
     */
    float getSteeringError() {
        uint8_t bits = readFilteredBits();
        bool l1 = (bits & (1 << 2)) != 0;
        bool r1 = (bits & (1 << 1)) != 0;

        float error = 0.0f;

        if (!l1 && !r1) {
            // Line đang nằm chuẩn ở giữa khe 2 mắt L1 & R1
            error = 0.0f;
        } else if (l1 && !r1) {
            // Lệch sang phải -> Vạch chạm L1 -> Bẻ lái Trái
            error = -1.0f;
        } else if (!l1 && r1) {
            // Lệch sang trái -> Vạch chạm R1 -> Bẻ lái Phải
            error = 1.0f;
        } else if (l1 && r1) {
            // Đi qua vạch ngang -> Cả 2 cùng chạm -> Giữ hướng thẳng
            error = 0.0f;
        }

        last_error = error;
        return error;
    }

    /**
     * @brief Hàm đếm vạch ngang với bộ lọc sườn xung (Rising-Edge Detector)
     *        Đảm bảo khi robot lăn bánh qua vạch ngang (dài 2-5cm) chỉ đếm đúng +1 lần duy nhất.
     * @return true nếu vừa bắt được một vạch ngang mới, false nếu không có hoặc vẫn đang nằm trên vạch cũ.
     */
    bool checkNewCrossLine() {
        bool current_cross = isCrossLine();

        // Phát hiện sườn lên: Từ không có vạch -> Bắt đầu chạm vạch ngang
        if (current_cross && !in_cross_line_state) {
            in_cross_line_state = true;
            return true; // Vừa chạm vạch mới!
        }
        
        // Khi xe đã lăn qua hẳn khỏi vạch ngang -> Reset trạng thái để sẵn sàng đếm vạch tiếp theo
        if (!current_cross && in_cross_line_state) {
            in_cross_line_state = false;
        }

        return false;
    }

    void resetLineCounterState() {
        in_cross_line_state = false;
    }

private:
    int pin_l2;
    int pin_l1;
    int pin_r1;
    int pin_r2;
    SensorPolarity sensor_polarity;
    uint8_t filtered_bits = 0;
    float last_error = 0.0f;
    bool has_detected = false;
    bool in_cross_line_state = false; // Chống đếm trùng khi đang trên vạch ngang
};

#endif // LINE_SENSOR_H
