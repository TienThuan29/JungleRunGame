# Checklist chơi thử: gameplay hoàn chỉnh

Claude không chạy game trên máy dev. Mọi mục dưới đây cần một người chơi thử. Mẹo: đặt `debug 1` trong `Resource/tuning.txt` để dùng F1 (bất tử), F2 (nhảy tới checkpoint kế), F3 (nhảy tới cuối thế giới), F5 (đọc lại tuning). Nhớ trả về `debug 0` trước khi merge.

## 1. Khởi động và menu
- [ ] Game mở, nhạc menu chạy, menu có 7 mục: Play, Story, Game Mode, Settings, Help, Donate, Exit.
- [ ] Chuột và phím Lên/Xuống + Enter đều chọn được; Esc ở menu chính không thoát game.
- [ ] Game Mode: Trái/Phải đổi độ khó, dòng mô tả (số mạng, checkpoint) đổi theo.
- [ ] Settings: chỉnh Music/Sounds nghe thấy thay đổi ngay; Fullscreen bật/tắt; F11 ở mọi màn hình.
- [ ] Fullscreen trên màn hình không phải 3:2: có viền đen, không méo, chuột bấm đúng nút.
- [ ] Help có thêm 5 dòng điều khiển bên phải, không đè lên hình có sẵn.
- [ ] Thoát game rồi mở lại: âm lượng, fullscreen, độ khó còn nguyên (file `save.txt` cạnh exe).

## 2. Chọn thế giới
- [ ] Play mở World Select, tiêu đề ghi độ khó; World 2 và 3 ghi "[locked - finish world N]".
- [ ] World 1 lần đầu chạy Intro; lần sau vào thẳng.

## 3. Cảm giác chơi (so với bản cũ trên màn 60 Hz)
- [ ] Chạy, nhảy, rơi giống bản cũ; trên màn 120/144 Hz không nhanh hơn.
- [ ] Kéo cửa sổ vài giây rồi thả: game không tua nhanh.
- [ ] Đứng yên sau khi chạy sang trái: nhân vật vẫn quay trái.
- [ ] Esc: Paused + gợi ý; nhạc dừng; đồng hồ HUD đứng yên; Esc tiếp tục; R chơi lại; X về menu.

## 4. Chiến đấu và vật phẩm
- [ ] J/K/F/Ctrl trái chém: có hình chém + tiếng vung; golem nhỏ vỡ vụn và biến mất.
- [ ] Golem cam (World 2, 3) nhanh gấp đôi.
- [ ] Bẫy tên bắn mỗi ~2,5 s; nhảy qua được; chém đứt được mũi tên.
- [ ] Golem lớn thỉnh thoảng vươn tay rồi ném tay về phía người chơi; chém hoặc né được.
- [ ] Trúng đòn: rung màn, hạt đỏ, nháy đỏ.
- [ ] HUD: mạng, bình thuốc, chìa khoá (World 1), đồng hồ, thanh tiến độ ở y≈56 không đè chữ, mũi tên đỏ nhấp nháy khi golem lớn ở gần ngoài màn hình.

## 5. Checkpoint
- [ ] Chạm ngôi sao: tiếng nhặt, ngôi sao chuyển xanh.
- [ ] Easy/Medium: chết (hết mạng, rơi vực, bị golem bắt) thì hồi sinh tại ngôi sao, đầy mạng, nháy mờ 2 s, golem ở phía sau xa.
- [ ] Hard: không có ngôi sao; chết là Bad End.

## 6. Ba thế giới
- [ ] World 1: 4 ngôi sao (lv01, lv03, lv05, lv09); vẫn có chìa khoá → bình thứ 5.
- [ ] World 2 "Temple Ruins": qua được mọi đoạn, nhất là lv18 (leo bậc qua vực) và lv19 (bục làm rào chắn).
- [ ] World 3 "Golem's Lair": qua được lv21, lv22.
- [ ] Mỗi thế giới nhặt đủ 5 bình → Good End.

## 7. Đấu boss (cuối World 3)
- [ ] Vào sàn đấu: camera trượt rồi khoá, golem trồi lên phía sau, thanh máu boss xuất hiện.
- [ ] Laser: có tiếng điện + quả cầu năng lượng trước; đứng dưới đất thì an toàn, nhảy thì trúng.
- [ ] Ném tay: tay lăn sát đất, nhảy qua được.
- [ ] Sau mỗi đòn boss đứng yên ~2 s; chém trúng thì boss nháy đỏ, mất 1 vạch máu.
- [ ] Đi vào thân boss: bị đau và đẩy sang phải.
- [ ] Chết trong sàn đấu (Easy/Medium): hồi sinh trong sàn đấu, boss đầy máu lại.
- [ ] Hết máu boss: boss vỡ vụn → True/Good End.

## 8. Kết thúc và kỷ lục
- [ ] Màn kết hiện Time, Potions x/5, Respawns, Best hoặc NEW RECORD!
- [ ] Sau 2 s: "R retry X menu" (+ "N next world" nếu thắng và còn thế giới sau).
- [ ] N vào thẳng thế giới kế với nhạc khác; World Select hiện best time đúng độ khó.
- [ ] Esc trên màn kết không mở pause.

## 9. Tay cầm (nếu có)
- [ ] Cắm khi đang chạy cũng nhận; cần trái/D-pad di chuyển; A nhảy, X chém, Start pause, Y chơi lại, Back về menu, RB thế giới kế, B quay lại ở menu.

## 10. Tuning
- [ ] Sửa `jump_vy -11` trong `tuning.txt`, bấm R: nhảy cao hơn.
- [ ] Ghi sai (`gravity abc`): hộp cảnh báo nêu dòng, game dùng giá trị mặc định.

Ghi lại số nào cần chỉnh (tên key + giá trị mới) để cập nhật `tuning.txt` mặc định.
