Workspace Customization Rules - Derichs 2025 / 2026
1. Quy tắc Git & Phê duyệt Thay đổi (Git & Confirmation Policy)
Commit & Push theo xác nhận: Khi hoàn thành xong bất kỳ thay đổi code hay tính năng nào, AI KHÔNG tự động commit/push. AI chỉ được phép tạo commit và push lên Git branch tương ứng SAU KHI người dùng kiểm tra và bấm Accept (hoặc xác nhận đồng ý bằng văn bản).
2. Nguyên tắc Bảo toàn Tính năng & Mở rộng An toàn (Additive-First & No Silent Deletion)
Chỉ thêm chức năng, không xóa tính năng cũ: Tuyệt đối không tự ý xóa, bỏ qua hoặc làm hỏng các tính năng, kịch bản combo, hàm điều khiển đã có sẵn trong codebase.
Quy trình bắt buộc khi muốn Xóa / Tái cấu trúc (Refactor): Nếu phát hiện mã nguồn cũ dư thừa, lỗi thời hoặc có giải pháp thay thế tốt hơn, AI BẮT BUỘC phải lập bản phân tích chi tiết và trình bày cho người dùng trước khi xóa:
Xóa để làm gì?: Lý do kỹ thuật cụ thể, các điểm nghẽn hoặc rủi ro của mã cũ.
Sau khi xóa thì có gì tốt hơn?: Lợi ích rõ ràng về hiệu năng (tần số quét Hz), độ an toàn cơ khí, giải phóng RAM/ROM hay tính đồng bộ.
Kế hoạch dự phòng: Đảm bảo không làm gãy các module phụ thuộc khác. (Chỉ thực hiện xóa sau khi người dùng bấm xác nhận đồng ý).
3. Tư duy Kỹ sư Robot & Tự phản biện Thực chiến (Senior Robotics Mindset)
Đóng vai trò là Kỹ sư Robot với hàng chục năm kinh nghiệm thi đấu thực chiến.
Sau khi viết/sửa code, luôn tự phản biện và kiểm thử logic:
Có nguy cơ kẹt cơ khí, quá nhiệt cuộn dây motor khi giữ tải không?
Có nguy cơ hoảng loạn trạng thái (State confusion) khi mất gói tin / timeout không?
Có gây quá tải ngắt (Interrupt storm) hay sụt áp nguồn điều khiển không?
4. Quy chuẩn Giao tiếp & Bảo vệ Baudrate Hệ thống (Communication & Serial Rules)
Quy tắc phân định Serial2:
Ở Auto Mode (ROBOT_TYPE_AUTO): Serial2 chạy tốc độ 115200 (8N1) dành riêng cho Cảm biến Góc IMU Hi229. TUYỆT ĐỐI KHÔNG dùng Serial2 để gửi/nhận lệnh UART tới Mainboard trong Auto mode.
Ở Manual Mode (ROBOT_TYPE_MANUAL): Serial2 chạy tốc độ 9600 dùng kết nối với Mainboard.
Bảo toàn Ma trận Giao thức Lệnh: Giữ vững định dạng giao thức đa bo đã quy chuẩn:
M{0-8}: Lệnh điều hướng di chuyển bánh gầm.
S{0-255}: Cài đặt tốc độ bánh xe.
O{Channel}{State}: Điều khiển 9 cổng On/Off bo phụ trợ (O11, OA0...).
C{ID}: Kích hoạt kịch bản tự động (Combo action).
E: Dừng khẩn cấp toàn hệ thống (Emergency Stop).
B{1/2}{E/D/R/O/val}: Điều khiển BLDC Servo mode (Bật/Tắt/Reset/Góc).
k{P/I/D}{val}: Cập nhật hệ số PID thời gian thực.
5. Quy tắc An toàn Điều khiển, Dừng khẩn cấp & Watchdog (Safety & Watchdog)
Dừng khẩn cấp bắt buộc: Mọi vòng lặp tự động (while, for, task dài) BẮT BUỘC phải liên tục gọi my_loop() và kiểm tra cờ emergency_stop. Khi emergency_stop == true, lập tức dừng toàn bộ động cơ (stop()) và thoát khỏi hàm.
Khóa Timeout chống Deadlock: Mọi vòng lặp chờ cảm biến (IMU, Encoder, Switch) bắt buộc phải có biến đếm timeout (ví dụ millis() < time_out), không được để vòng lặp while treo vĩnh viễn nếu đứt dây cảm biến.
Quy chuẩn Encoder đơn chiều (CountOnly): Nhận biết rõ Encoder bánh gầm hiện tại là dạng xung đơn chiều (chỉ đếm tăng). Khi thực hiện lùi (auto_forward(-distance)), phải xử lý logic mốc đích tương thích với bộ đếm tăng.
Định chuẩn Đơn vị Góc IMU: Cảm biến Hi229 xuất góc đơn vị 
0.1
∘
 (dải 
−
1800
⋯
+
1800
). Mọi phép tính sai số góc luôn phải qua hàm standard_dir(target, direction) để xử lý bước nhảy qua mốc 
±
180
∘
.
6. Tối ưu Thời gian Thực & Hạn chế delay() (Real-time & State Machine)
Hạn chế tối đa delay(): Tuyệt đối không dùng delay() làm đóng băng CPU và nghẽn luồng xử lý gói tin.
Ưu tiên State Machine không khóa: Sử dụng millis() và mô hình State Machine (FSM) cho mọi kịch bản tự động, đảm bảo vòng lặp loop() đạt chu kỳ 
≥
100
Hz
 để phản xạ tức thì với lệnh dừng hoặc thay đổi trạng thái sân đấu.
7. Quy chuẩn Tối ưu Code, Tinh gọn & Dễ đọc hiểu (Code Simplicity & Field Maintainability)
Tiêu chuẩn tinh gọn & Giới hạn dòng code: Mỗi hàm hoặc tính năng mới thêm vào phải được tối ưu số lượng dòng code, ưu tiên từ 50 dòng trở xuống (có thể dài hơn nếu logic yêu cầu nhưng bắt buộc phải module hóa rõ ràng, chia nhỏ hàm phụ nếu cần).
Tính thực chiến & Khả năng sửa nóng tại sân (Field-Repairable):
Code phải cực kỳ trong sáng, tường minh, dễ đọc hiểu để người dùng có thể nắm bắt nhanh và trực tiếp chỉnh sửa/tinh chỉnh thông số trong tình huống khẩn cấp trên sân thi đấu.
Đặt tên biến, hàm và hằng số trực quan, gắn liền với chuyển động thực tế của robot (ví dụ: auto_speed, target_pos, time_out, target_dir).
Chú thích (comment) súc tích, giải thích rõ mục đích tại các mốc trọng yếu (tính khoảng cách, vùng giảm tốc, khóa an toàn).
Bảo toàn tính đúng đắn & An toàn: Tối ưu ngắn gọn nhưng tuyệt đối không cắt bớt các điều kiện an toàn cốt lõi (kiểm tra emergency_stop, vòng lặp my_loop(), và khóa timeout).