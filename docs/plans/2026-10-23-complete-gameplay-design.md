# Jungle Run: nâng cấp thành gameplay hoàn chỉnh

Trạng thái: đã chốt sau 3 review (mục 10 ghi phân xử; mục 10 thắng khi mâu thuẫn với mục 4). Triển khai trên `feat/complete-gameplay`.

## 1. Tóm tắt cách hiểu (đã xác nhận)
- Nâng game từ "một màn chạy dài, 3 kết thúc" thành game hoàn chỉnh trên 4 mảng: vòng chơi lõi, trình bày, tiến trình, nội dung.
- Người chơi tải zip từ README; user là người chơi thử duy nhất, Claude không chạy game.
- Một branch lớn, mỗi tính năng một commit, user chơi thử ở cuối.
- Giữ luật code: C++17, W4 không warning, RAII, không `new`/`delete`, không exception điều khiển luồng, không thêm thư viện.
- Không làm: nhiều người chơi, mạng, đăng nhập, installer, x86, macOS/Linux, thành tích (achievements), Ledge Grab.

## 2. Hiện trạng liên quan
- Vật lý người chơi tính theo frame (`dx += 0.3`, `g = 0.25` mỗi frame), boss/golem nhỏ theo `dt`. VSync bật nên tốc độ đổi theo tần số màn hình.
- Thời gian trong game dùng `SDL_GetTicks64()` (bất tử, boss delay, màn kết, animation bẫy).
- `World` sở hữu 1 run: Player, GigaGolem, Level, Parallax, Hud. `GameplayScene` chỉ bơm input và gọi `step/render`.
- Entity trong lưới: Tile, Trap (bảng `kTraps`), Pickup (13/14/21), SmolGolem (19), Decoration (22). Mã tối đa 22.
- Course: `Resource/Levels/course.txt` + `lvNN.txt`. Một course duy nhất.
- Menu 960x668, gameplay 1200x800 bằng cách đổi kích thước cửa sổ. Chỉ chuột trong menu.
- Asset sẵn có chưa dùng: Golem sheet có hàng vươn tay bắn (y=200, 9 frame), hàng vỡ vụn (y=700, 10 frame), hàng trồi lên (y=800, 4 frame), hàng bị đánh (y=400, 7 frame); `Laser_sheet.png` 300x1500 (15 frame 300x100: 8 frame tụ năng lượng, 7 frame tia); `arm_projectile_glowing.png` 300x300 (6 frame 100x100); Arrow Trap (51 frame 32x32) + `Arrow.png` 29x3; item sheet hàng 8 = ngôi sao; nhạc In Game 1/2, Storyline 1/2, Main Menu.mp3 chưa dùng.
- SDL 2.26.4, SDL_mixer 2.6.2.

## 3. Giả định
1. Windows x64, SDL2 như hiện tại. File lưu và file tuning là text, parser tự viết.
2. File lưu hỏng hoặc thiếu → dùng mặc định, ghi log, không crash. Không mạng.
3. `tools/parity` không còn trong git (chỉ còn rác build) → không có gì để xoá; ghi chú trong README rằng parity với bản C không còn áp dụng.
4. Asset tải về chỉ CC0: hiệu ứng chém của tbbk (OpenGameArt, canvas 64x47) và gói tiếng vung kiếm "Swishes Sound Pack" (artisticdude, OpenGameArt). Giữ `LICENSE.txt` cạnh file, ghi công trong README. Không thêm pack có điều khoản "không phân phối lại".
5. `main` vẫn là code C; branch này chồng lên 3 branch chưa merge.
6. User chơi thử một lần ở cuối; bản Debug có phím tắt để thử nhanh.
7. 60 tick/giây là chuẩn cảm giác chơi (giữ nguyên mọi hằng số hiện có).

## 4. Thiết kế

### 4.1 Vòng lặp và thời gian
- `GameplayScene::run`: tích luỹ `dt`, chạy `world.step(1/60)` mỗi tick, tối đa 5 tick/frame (kẹt máy không nhảy vọt). Render mỗi frame.
- `World` giữ `time_` (giây, cộng mỗi tick). Mọi mốc thời gian gameplay (bất tử, boss delay, màn kết, bẫy, cooldown) dùng `time_`. `animFrame()` (hoạt ảnh thuần vẽ) vẫn dùng đồng hồ máy.
- `Trap::animate(nowMs)` nhận `worldMs` thay vì `SDL_GetTicks64()`.

### 4.2 Tuning: `Resource/tuning.txt`
- Dạng `key value` mỗi dòng, `#` chú thích. Struct `Tuning` (trong `game/Tuning.h`) với mặc định = hằng số hiện tại. Key lạ → log cảnh báo; key thiếu → mặc định. Đọc trong `GameplayScene` ctor (sửa file, chơi lại là có hiệu lực).
- Key: `run_accel 0.3`, `run_max 2.5`, `friction 0.8`, `jump_vy -9`, `gravity 0.25`, `immortal_sec 1`, `respawn_immortal_sec 2`, `boss_speed 75`, `boss_delay_sec 0.5`, `boss_touch 0.75`, `boss_fire_sec 4`, `boss_fire_range 900`, `boss_warn_px 700`, `respawn_boss_gap 1500`, `projectile_speed 260`, `projectile_life_sec 4`, `arrow_speed 320`, `arrow_every_sec 2.5`, `smol_speed 40`, `smol_max 120`, `smol_detect 150`, `smol_fast_mul 2`, `attack_sec 0.2`, `attack_cooldown_sec 0.45`, `attack_w 70`, `attack_h 60`, `boss_hp 5`, `boss_idle_sec 1`, `boss_charge_sec 0.8`, `boss_beam_sec 0.6`, `boss_open_sec 2`, `beam_low_dy 180`, `beam_high_dy 90`, `beam_h 40`, `shake_hit 8`, `shake_decay 20`.
- `Player`, `GigaGolem`, `SmolGolem`, `ArrowTrap`, `Projectile` nhận `const Tuning&` qua `World::tuning()`.

### 4.3 Pause
- ESC (hoặc Start tay cầm) trong gameplay: `paused_` trong `GameplayScene`. Khi pause: không `step`, vẽ world rồi phủ đen 60% + chữ "Paused" + "ESC continue   R restart   X menu". `Mix_PauseMusic/ResumeMusic`.
- Không có menu con, không vào Settings từ pause.

### 4.4 Checkpoint (mã 23, hình ngôi sao item hàng 8)
- `Checkpoint : Entity`: chạm → `World::setCheckpoint(x, y)`, tiếng nhặt đồ, sao đổi màu (color mod xanh). Chạm lại không làm gì.
- Khi chết (mạng 0, rơi, boss chạm) và độ khó khác Hard và đã có checkpoint: hồi sinh thay vì Bad End: dịch người chơi tới checkpoint, `dx=dy=0`, mạng = tối đa, bất tử `respawn_immortal_sec`, boss lùi về `checkpoint.x - respawn_boss_gap` (không nhỏ hơn 0), đạn/tên trên màn xoá, `respawns_++`. Bình thuốc, chìa khoá giữ nguyên.
- Hard hoặc chưa có checkpoint: Bad End như cũ.

### 4.5 Settings + file lưu
- `Settings` (trong `core/Game.h`) thêm: `music 0..128` (mặc định 40), `sfx 0..128` (96), `fullscreen bool`, `unlockedWorlds 1..3`, `best[world][difficulty]` ms (0 = chưa có).
- `core/SaveFile.h/.cpp`: `load(Settings&)` / `save(const Settings&)` tới `<SDL_GetBasePath()>save.txt`, dạng `key value`. Lưu ngay khi thay đổi (Settings scene, kết thúc run có kỷ lục, mở khoá).
- Âm lượng: `Game::applyAudio()` gọi `Mix_VolumeMusic(music)` và `Mix_Volume(-1, sfx)`. Bỏ các `Mix_VolumeMusic(32/64)` rải trong scene. Volume riêng từng chunk (16..96) giữ làm tỉ lệ tương đối.
- `SettingsScene` (960x668, nền `gamemode.bmp`): 3 hàng `Music  << 40 >>`, `SFX  << 96 >>`, `Fullscreen << On/Off >>`, nút Return. Bước 8. Vào từ Menu ("Settings" thay chỗ "Donate"? Không: Donate giữ, menu có 7 mục).

### 4.6 Fullscreen và kích thước logic
- Cửa sổ cố định 1200x800. `Game::resize(w,h)` đổi thành `SDL_RenderSetLogicalSize(r, w, h)`; menu 960x668 được scale có viền đen (SDL tự quy đổi toạ độ chuột).
- Fullscreen: `SDL_SetWindowFullscreen(w, SDL_WINDOW_FULLSCREEN_DESKTOP | 0)`. F11 bật/tắt ở mọi scene (xử lý chung trong `Game::handleGlobal(e)`, được gọi trong cả 3 vòng lặp), lưu vào settings.

### 4.7 Tay cầm và điều hướng bàn phím
- `sdl::Session` thêm `SDL_INIT_GAMECONTROLLER`; `Game` mở tay cầm đầu tiên khi `SDL_CONTROLLERDEVICEADDED`, đóng khi `REMOVED` (`ControllerPtr` RAII).
- `Game::handleGlobal(SDL_Event&)` dịch nút tay cầm thành sự kiện phím: A→Space, B→Escape, X→J, Start→Escape, D-pad→mũi tên. Mọi scene nhận phím như nhau.
- Gameplay đọc thêm trục trái/D-pad cho trái/phải/xuống.
- Menu, GameMode, Settings, WorldSelect: thêm `selected_` + Up/Down/Enter/Space; nút đang chọn dùng màu "hot". `Button::setHot(bool)`.

### 4.8 Hiệu ứng (code vẽ, không asset mới trừ slash)
- `ScreenShake`: biên độ giảm theo `shake_decay`; render world với `SDL_RenderSetViewport` lệch ngẫu nhiên, reset sau. Kích hoạt khi trúng đòn, khi boss trúng đòn, khi boss chết.
- `Particles` trong `World`: mảng cố định 256 phần tử `{x,y,dx,dy,life,kind}`. Kind: Sparkle (sheet sparkle, khi nhặt đồ), Dust (hình chữ nhật nâu nhỏ khi tiếp đất/chạy), Hit (chữ nhật đỏ). Vẽ sau entity, trước HUD.
- Fade-in 0.3 s khi vào scene (phủ đen alpha giảm) ở `Scene::run` và `GameplayScene::run`. Fade-out: `Mix_FadeOutMusic(300)` khi rời scene có nhạc.

### 4.9 HUD và màn kết
- Thêm: icon chìa khoá khi có chìa; thanh tiến độ trên cùng (vạch người chơi, vạch boss, điểm finish/arena); cảnh báo boss: mũi tên đỏ nhấp nháy mép trái khi boss cách < `boss_warn_px`; đồng hồ mm:ss.t góc phải; thanh máu boss khi đấu boss.
- Màn kết (vẫn trong `World::render`): tiêu đề như cũ + dòng `Time`, `Potions x/5`, `Respawns`, `Best` (và "NEW RECORD" nếu phá kỷ lục) + gợi ý `R retry   X menu   N next world` (N chỉ khi còn thế giới sau và kết thúc không phải Bad).
- Điểm số = thời gian hoàn thành (thấp hơn là tốt). Không có điểm cộng dồn riêng.

### 4.10 Nhạc
- `worlds.txt` chỉ định nhạc in-game 1/2/3 cho mỗi thế giới. Story scene: `Storyline 1.mp3`. World Select: `Storyline 2.mp3`. Menu giữ `MainMenu2.mp3`.

### 4.11 Thế giới và course
- `Resource/Levels/worlds.txt`: mỗi dòng `<courseFile> <musicIndex 1-3> <tiêu đề...>`; tối đa 8. Ví dụ: `course1.txt 3 The Relic`.
- Đổi tên `course.txt` → `course1.txt`. Thêm `course2.txt`, `course3.txt` dùng chung thư mục lưới; lưới mới `lv16.txt`..`lv31.txt` (khoảng 16 lưới mới), dùng mã mới 23-27. Course 2 và 3 cũng trộn lại lưới cũ.
- `course.txt` thêm dòng `arena X` (tuỳ chọn): tới `x >= X` thì vào đấu boss (xem 4.13); `finish` vẫn bắt buộc và phải > X + 1200. Course 3 có arena.
- Mở khoá: xong thế giới N với kết thúc True/Good → `unlockedWorlds = max(., N+1)`, lưu. Kỷ lục theo (world, difficulty) chỉ ghi khi kết thúc không phải Bad.
- `WorldSelectScene` (960x668, nền `ChooseBG.bmp`): tiêu đề "Select World", 3 hàng `1  The Relic   best 01:23.4` / `2  [locked]`, Return. Chọn → `settings.world = i`; thế giới 1 lần đầu đi qua Intro, các lần khác vào thẳng Gameplay (lưu `introSeen`).
- Menu: "Single Player" → "Play" (vào World Select). Thêm "Settings".

### 4.12 Chiến đấu
- Tấn công: phím J (K cũng được) / nút X. `Player::attack()` khi hết cooldown: `attacking_` trong `attack_sec`, hitbox trước mặt `attack_w x attack_h` theo hướng `flip`. Vẽ slash (sheet CC0, lật theo hướng) ở hitbox. Tiếng swish.
- Mục tiêu: `Entity::hit()` ảo (mặc định không làm gì). `SmolGolem::hit()` → `dying_`, phát hàng vỡ vụn (y=700, 10 frame, 0.5 s) rồi `isDead()` → Level bỏ qua va chạm và xoá sau khi hoạt ảnh xong. `Projectile` bị chém → huỷ + hạt. Boss: xem 4.13.
- `Projectile` (không nằm trong lưới, `World::projectiles_`): `{box, dx, dy, life, kind: Arm|Arrow}`, gây sát thương khi chạm (bất tử thì không), huỷ khi hết `life` hoặc ra ngoài course. Arm: 6 frame 100x100 scale 50x50. Arrow: 29x3 scale x2, lật theo hướng.
- Boss khi đuổi: mỗi `boss_fire_sec` nếu cách người chơi < `boss_fire_range` thì phát hoạt ảnh vươn tay (y=200) 0.4 s rồi bắn Arm về phía người chơi theo đường thẳng ngang ở độ cao giữa boss.

### 4.13 Đấu boss (course có `arena X`)
- Khi `player.x >= X`: `arena_ = true`, camera khoá `scrollX = -X`, người chơi không ra khỏi `[X, X+1200-w]`. Boss dịch tới `x = X + 60`, `y = mặt đất - 250` (đứng trên nền), `bossHp = boss_hp`, trồi lên (y=800, 4 frame) rồi vào máy trạng thái.
- Trạng thái: `Idle(boss_idle_sec)` → chọn ngẫu nhiên `Laser` hoặc `Throw` → `Charge(boss_charge_sec)`: Laser = hoạt ảnh vươn tay (sheet hàng y=200, 9 frame) + quả cầu tụ năng lượng (laser frame 1-8) tại đầu cánh tay (boss.x+345, boss.y+55 với boss vẽ 380x250 từ crop 76x50 của ô); Throw = hoạt ảnh ném tay (hàng y=400, 7 frame) → `Fire`: Laser = tia frame 9-14 lặp `boss_beam_sec`, vẽ từ đầu tay tới mép phải (ô đầu 300 px, các ô sau dùng nửa phải 150 px của sheet), vùng sát thương `{x: đầu tay → mép phải, y: ey-15, h: 30}`; Throw = một Arm projectile bay sát đất về phía người chơi → `Open(boss_open_sec)`: boss đứng yên (hàng y=0 idle), nhận đòn → về Idle.
- Laser ở ngang ngực boss (y ≈ 446 khi boss đứng trên nền), cao hơn người chơi đang đứng (577..641) nhưng trúng người đang nhảy (đỉnh nhảy ≈ 415..485) → phải đứng yên. Arm bay sát đất → phải nhảy. Hoạt ảnh tụ năng lượng / ném tay cho biết trước. Bỏ các key `beam_low_dy`, `beam_high_dy`; thay bằng `beam_dy 55`, `beam_h 30`.
- Kích thước đo được: slash sheet 192x141 = 3x3 frame 64x47 (dùng frame 1-7, vẽ 96x70); Arm projectile 6 frame, phần sáng tại (55..100, 25..50) trong ô 100 → vẽ 90x50, hitbox 70x30; Arrow Trap bắn sang trái, tên xuất hiện ở frame 9-10 của 51 frame.
- Boss chỉ nhận đòn khi `Open`; trúng → `bossHp--`, nháy đỏ (color mod), rung màn, hạt. `bossHp == 0` → `Dying` (hàng vỡ vụn 1 s) → `isOver_ = true` với `status` theo bình thuốc (True/Good), mở khoá thế giới sau. Boss không chạm (touches) người chơi trong arena.
- Boss chạm trong arena không gây chết; sát thương chỉ từ laser và Arm.

### 4.14 Entity mới trong lưới
| Mã | Entity | Ghi chú |
|---|---|---|
| 23 | Checkpoint | ngôi sao, item hàng 8 |
| 24 | ArrowTrap bắn sang trái | sheet Arrow Trap lv1, tên bay từ frame bắn, mỗi `arrow_every_sec` |
| 25 | ArrowTrap bắn sang phải | lật ngang |
| 26 | SmolGolem nhanh | `smol_fast_mul`, color mod cam; `kMaxTileCode = 26` |
- `Entity` thêm: `virtual void update(World&, float dt)` (ArrowTrap spawn tên qua `World::spawnProjectile`), `virtual void hit(World&)`, `virtual bool isDead()`. `Level::collideSegments` bỏ qua entity đã chết; `Level::prune()` xoá sau hoạt ảnh.

### 4.15 Hỗ trợ thử (chỉ `_DEBUG`)
- F1 bất tử bật/tắt, F2 nhảy tới checkpoint kế, F3 nhảy tới arena/finish, F5 nạp lại `tuning.txt`. Hiện chữ "DEBUG" góc dưới khi bất tử.

### 4.16 Help và README
- Help scene: thêm dòng chữ điều khiển (J chém, ESC pause, F11 fullscreen, tay cầm) và icon ngôi sao/arrow trap.
- README: điều khiển, `tuning.txt`, `worlds.txt`, mã lưới 23-26, `arena`, `save.txt`, ghi công asset CC0 mới.

## 5. Luồng dữ liệu
Menu → Play → WorldSelect(world) → [Intro lần đầu] → Gameplay: đọc `tuning.txt`, `worlds.txt`, `courseN.txt` → `World(assets, course, tuning, difficulty)` → tick 60 Hz: input → Level.update/collide → projectiles → boss → physics → particles → HUD. Kết thúc: ghi kỷ lục/mở khoá → `SaveFile::save`.

## 6. Lỗi và biên
- Thiếu `tuning.txt`/`worlds.txt`: mặc định (worlds mặc định = `course1.txt 3 The Relic`), log. Course lỗi: hộp thoại như hiện nay, về menu.
- `arena X` không có trong course → không có đấu boss, finish như cũ. `arena` + `onkey` dùng chung được.
- Hồi sinh khi boss đã vào arena: boss ở lại arena, người chơi về checkpoint (checkpoint trong arena không hợp lệ: loader từ chối checkpoint có x >= arena X).
- Tay cầm rút ra giữa chừng: input về 0.
- Save file ghi hỏng giữa chừng: lần sau đọc lỗi → mặc định.
- Fullscreen trên màn hình tỉ lệ khác: viền đen, không méo.

## 7. Kiểm chứng
- Build Debug + Release x64, 0 warning. Mỗi commit build.
- Tự kiểm bằng code không cần cửa sổ: parser `Tuning`, `SaveFile`, `worlds.txt`, course `arena` (một `tools/selfcheck.cpp` nhỏ, assert, chạy trong Debug? Không: giữ YAGNI, kiểm bằng cách đọc file thật lúc khởi động và log lỗi; user chạy Debug để xem log).
- User: checklist chơi thử theo từng mục 4.1-4.15, mở trong `docs/plans/...-complete-gameplay-checklist.md`.

## 8. Rủi ro
1. Không ai chơi thử trước khi xong: cảm giác chém/laser/level mới có thể lệch; mọi số đều ở `tuning.txt` để chỉnh nhanh.
2. Level mới thiết kế mù: có thể không qua được; user sửa lưới text, không cần build.
3. Bước cố định 60 Hz đổi hành vi va chạm tinh vi so với trước ở màn hình 144 Hz (trước chạy nhanh hơn 2.4x; giờ đúng tốc 60 Hz). Đây là chủ đích.
4. `SDL_RenderSetLogicalSize` cho menu 960x668 có viền đen; chuột do SDL quy đổi.
5. Diff lớn (ước ~2.500 dòng mới); review theo từng commit.

## 9. Decision log
| # | Quyết định | Phương án khác | Lý do |
|---|---|---|---|
| 1 | Bước cố định 60 Hz, giữ hằng số per-frame | Đổi mọi công thức sang dt | Giữ nguyên cảm giác đã có, diff nhỏ, không phải chỉnh lại số |
| 2 | Giữ nhân vật, thêm slash VFX CC0 vẽ đè | Thay cả nhân vật bằng pack có đòn (rvros) | Pack kia cấm phân phối lại (repo public), menu/Help đang dùng hình nhân vật cũ |
| 3 | Boss bắn Arm + arena laser 2 độ cao, chém khi hở | Chỉ đuổi + bắn | Asset laser/arm sẵn có; đấu boss là thứ user yêu cầu; 2 độ cao tạo quyết định nhảy/đứng |
| 4 | Checkpoint hồi mạng đầy, boss lùi | Giữ mạng còn lại | Ít trạng thái, Hard vẫn 1 mạng không checkpoint |
| 5 | Điểm = thời gian hoàn thành | Điểm cộng dồn (bình thuốc, khoảng cách) | Một con số dễ hiểu, kỷ lục có ý nghĩa; bình thuốc đã quyết định kết thúc |
| 6 | 3 thế giới dùng chung thư mục lưới, `worlds.txt` | Mỗi thế giới một thư mục | Không phải di chuyển 17 file, loader đổi tối thiểu |
| 7 | Tay cầm dịch sang sự kiện phím | Xử lý tay cầm riêng từng scene | Một chỗ, mọi scene được hỗ trợ |
| 8 | Logical size, cửa sổ cố định 1200x800 | Giữ đổi kích thước cửa sổ, fullscreen riêng | Fullscreen và windowed cùng một đường vẽ |
| 9 | Pause chỉ overlay chữ | Menu pause có nút | YAGNI; R/X đã có sẵn từ màn kết |
| 10 | Save `key value` cạnh exe | %APPDATA%, JSON | Game chạy từ zip, không cài; không thêm thư viện |
| 11 | Fade-in khi vào scene, fade-out chỉ nhạc | Fade-out hình (scene phải vẽ tiếp sau `go`) | Vòng lặp scene hiện tại không hỗ trợ; đủ cảm giác chuyển cảnh |
| 12 | Golem nhanh là biến thể duy nhất | Golem nhảy, golem bay | Không có asset bay; nhảy cần logic mặt đất riêng |
| 13 | Debug keys trong `_DEBUG` | Cheat trong Release | Không lộ cheat cho người chơi |
| 14 | Intro chỉ chạy lần đầu | Mỗi lần Play | Đỡ phiền khi chơi lại |

## 10. Phân xử review (Arbiter)

Ba reviewer: Skeptic (S), Constraint Guardian (C), User Advocate (U). Quyết định dưới đây ghi đè mục 4 khi khác nhau.

### Chấp nhận, sửa thiết kế
| Nguồn | Vấn đề | Sửa |
|---|---|---|
| S1 | Input áp dụng mỗi frame, physics mỗi tick → cảm giác vẫn phụ thuộc FPS | `World::tick(InputState)` gọi `player.input()` trong từng tick; scene chỉ lấy mẫu phím mỗi frame |
| S2, C3 | Kẹt cửa sổ → tua nhanh | Kẹp chính accumulator ở 5 tick |
| S3 | `Player::hurt` / bất tử / boss delay / màn kết dùng đồng hồ máy | Mọi mốc gameplay dùng `World::time()` (= số tick / 60); `animFrame` chỉ cho hoạt ảnh trang trí |
| S4, U6 | Vùng miễn nhiễm cạnh boss, cánh tay bay ngang hông không trúng người đứng | Thân boss gây sát thương + đẩy lùi khi chạm; vùng laser bắt đầu từ mép thân; cánh tay trong sàn đấu bay sát đất; cánh tay khi đuổi nhắm thẳng vào người chơi |
| S5 | Đứng yên thì nhân vật quay phải, không chém được sang trái | Đứng yên giữ hướng cũ (bỏ quirk `flip = 0`); đòn chém theo hướng đang nhìn |
| S6 | Laser thấp chỉ có khe nhảy 167 ms | Bỏ laser thấp. Laser ngang ngực (y≈476) = đứng yên dưới đất; cánh tay sát đất = nhảy |
| S7, U7 | Hồi sinh trong sàn đấu chưa định nghĩa | Chết trong sàn đấu (Easy/Medium) → hồi sinh trong sàn đấu, boss hồi đầy máu và trồi lên lại; Hard → Bad End |
| S8 | Camera giật 350 px khi vào sàn đấu | Camera trượt dần (10%/tick) tới vị trí khoá |
| S9, C4 | Rung màn bằng viewport phá letterbox | Rung = cộng độ lệch vào `scrollX` khi vẽ, không đụng viewport |
| S10, U1, U2 | Tay cầm thiếu nút; B trên menu thoát game | A=Space, B=Esc, X=J, Y=R, Back=X, Start=Esc, RB=N, D-pad=mũi tên; event giả có cả `sym` và `scancode`; di chuyển đọc trực tiếp D-pad/cần trái (deadzone 12000); ESC ở menu chính không thoát nữa |
| S11 | `Mix_Volume` nhân với volume từng chunk | SFX mặc định 128 (giữ âm lượng cũ), nhạc mặc định 32 (như cũ) |
| S12, C5 | `SDL_GetBasePath` có thể NULL, ghi file hỏng | NULL → `save.txt` ở thư mục hiện tại; ghi `save.txt.tmp` rồi `std::filesystem::rename` |
| S13 | Menu 960x668 scale 1.197x xấu | Cửa sổ vẫn đổi kích thước như cũ; logical size chỉ để fullscreen scale có viền |
| S14 | Loader chưa kiểm sàn đấu | `arena` (không tham số) = vị trí con trỏ hiện tại; bắt buộc có ground phủ kín 1200 px sau đó; không có checkpoint (23) từ `arena` trở đi; course có `arena` thì không có `finish`; tên thế giới không chứa `#`; tối đa 3 thế giới |
| S15 | Hoạt ảnh một lần chạy theo đồng hồ chung | Theo thời gian trong trạng thái của boss/golem |
| S16 | `Entity::update` trùng `preCollide` | Đổi tên `preCollide` thành `update(const Player&, World&, float)`; cảnh báo boss chỉ khi boss ra khỏi màn hình mà ở gần (`boss_warn_px` 400) |
| C1 | Đổi tên `course.txt` làm hỏng CI | Không đổi tên: thế giới 1 vẫn là `course.txt`. CI kiểm thêm `worlds.txt`, `tuning.txt` và build cả Debug |
| C2 | Giấy phép | Thêm mục ghi công (CREDITS) trong README, liệt kê cả pack có sẵn và điều khoản của chúng |
| C6 | `tuning.txt` là dữ liệu người dùng | Bảng `{tên, trường, min, max}`; giá trị sai/ngoài khoảng → giữ mặc định và hiện hộp cảnh báo liệt kê dòng lỗi; `std::from_chars`; tối đa 64 projectile; bẫy tên chỉ bắn khi cách người chơi < 1300 px |
| C7 | Mask `Uint32` với 51 frame | `ArrowTrap` là lớp riêng; sheet thật ra là 17 frame 96x32 (tên bay ra ở frame 9) |
| C8 | Texture chữ | Đồng hồ cache theo 1/10 giây; chữ màn kết dựng một lần |
| C10, C11 | Bảo trì / W4 | Đăng ký mọi file mới trong `.vcxproj` và `.filters`; Debug và Release đều 0 warning |
| U3 | Hard vẫn hiện ngôi sao | Hard không tạo checkpoint; World Select ghi "Hard: no checkpoints" |
| U4 | Không thấy độ khó khi chọn thế giới | Tiêu đề World Select ghi độ khó; kỷ lục là của độ khó đang chọn |
| U5 | Báo trước laser | Tiếng điện khi tụ năng lượng + quả cầu năng lượng ở đầu tay; ném tay có hoạt ảnh vung |
| U8 | Người chơi thử chỉ có bản Release | `tuning.txt` được đọc lại mỗi lần R/thế giới mới; phím F1-F3, F5 bật bằng `debug 1` trong `tuning.txt` (hoặc bản Debug) |
| U9 | Đơn vị khó hiểu | Mỗi key trong `tuning.txt` có chú thích đơn vị |
| U10 | Settings | Hiện phần trăm; mỗi bước phát tiếng click |
| U11 | ESC trên màn kết | Bỏ qua ESC khi đã kết thúc |
| U12 | Fade làm nháy menu | Menu 0.15 s, gameplay 0.3 s |
| U13 | Thanh tiến độ đè HUD | Thanh ở y=56 |
| U14 | "[locked]" | "[locked - finish world N]" |
| U15 | Phím chém một tay | J, K, F, Ctrl trái |

### Bác bỏ / hoãn
- **C2 (pack có điều khoản "không phân phối lại" đã có trong repo):** đã tồn tại từ trước, ngoài phạm vi; chỉ ghi rõ trong CREDITS.
- **C8 (bỏ qua update cho segment ngoài màn hình):** chưa cần, vài nghìn lời gọi ảo mỗi frame.
- **Nhạc Storyline cho Story / World Select (mục 4.10):** bỏ; nhạc menu đang phát xuyên suốt các menu, đổi nhạc giữa chừng gây giật. Chỉ dùng In Game 1/2/3 cho 3 thế giới.
- **Hạt bụi khi tiếp đất (4.8):** bỏ; hạt chỉ dùng cho nhặt đồ, trúng đòn, golem vỡ, boss trúng đòn.

### Kết luận Arbiter
APPROVED với các sửa ở trên. Rủi ro còn lại: cảm giác đánh boss và 2 course mới chưa ai chơi thử; mọi con số nằm trong `tuning.txt` và file lưới.
