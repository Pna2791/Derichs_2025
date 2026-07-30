# Workspace Customization Rules - Derichs 2025

- **Tự động Commit & Push**: Mỗi khi hoàn thành xong bất kỳ thay đổi code hay tính năng nào, AI bắt buộc phải tự động tạo commit và push lên Git branch tương ứng mà không cần đợi nhắc.
- **Quy tắc bảo vệ Baudrate & Serial2**: Ở Auto mode, `Serial2` dành riêng cho Hi229 (115200). Không dùng `Serial2` cho UART gửi lệnh tới Mainboard trong Auto mode.
- **Dừng khẩn cấp**: Vòng lặp tự động nào cũng phải gọi `my_loop()` và kiểm tra `emergency_stop`.
- **Hạn chế delay()**: Ưu tiên sử dụng State Machine với `millis()` thay vì dùng `delay()` làm đơ mạch.
