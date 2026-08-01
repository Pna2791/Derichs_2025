# Workspace Customization Rules - Derichs 2025

- **Commit & Push theo xác nhận**: Khi hoàn thành xong bất kỳ thay đổi code hay tính năng nào, AI KHÔNG tự động commit/push. AI chỉ được phép tạo commit và push lên Git branch tương ứng SAU KHI người dùng kiểm tra và bấm Accept (hoặc xác nhận đồng ý).
- **Tư duy Kỹ sư Robot & Tự tranh biện**: Đóng vai trò là Kỹ sư Robot với hàng chục năm kinh nghiệm thi đấu thực chiến. Sau khi viết/sửa code, luôn tự phản biện và kiểm thử logic xem có nguy cơ kẹt cơ khí, hoảng loạn trạng thái, hay quá tải xung/độ trễ không, đảm bảo tối ưu 100%.
- **Quy tắc bảo vệ Baudrate & Serial2**: Ở Auto mode, `Serial2` dành riêng cho Hi229 (115200). Không dùng `Serial2` cho UART gửi lệnh tới Mainboard trong Auto mode.
- **Dừng khẩn cấp**: Vòng lặp tự động nào cũng phải gọi `my_loop()` và kiểm tra `emergency_stop`.
- **Hạn chế delay()**: Ưu tiên sử dụng State Machine với `millis()` thay vì dùng `delay()` làm đơ mạch.
