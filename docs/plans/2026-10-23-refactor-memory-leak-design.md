# Jungle Run — Thiết kế refactor & sửa memory leak

Ngày: 2026-10-23 · Trạng thái: đã triển khai (branch `refactor/scene-loop-memleak`), chờ kiểm thử runtime

## 1. Bối cảnh

Game C/SDL2 (~4.900 dòng). RAM tăng không giới hạn khi chơi vài phút. Nguyên nhân tìm được khi đọc code:

| # | Leak | Vị trí | Mức độ |
|---|------|--------|--------|
| 1 | `TTF_OpenFont` + 2 texture tạo **mỗi frame**, không free | `render.c` `render_status()` | Chính — ~3.600 font/phút ở 60fps |
| 1b | Font 108pt + texture tạo mỗi frame ở màn kết thúc, thêm 1 font sau 2s | `common_functions.c` `Over()` | Cao |
| 2 | Retry (R) gọi `load_game()` nạp lại toàn bộ texture/sound/music, init lại audio/TTF | `common_functions.c` `Over()` → `load.c` | Cao |
| 3 | Màn hình gọi nhau đệ quy, không return; mỗi lần tạo window/renderer/font/texture mới, chỉ destroy window | `menu.c`, `help.c`, `preGameplay.c` | Cao, tăng theo số lần chuyển màn |
| 4 | Texture tạo mới mỗi `SDL_MOUSEMOTION`, `TTF_OpenFont` mỗi event, `getText()` không free | `menu.c`, `help.c`, `checkAccount.c` | Trung bình |
| 5 | `UpdateText()` tạo texture mỗi lần thêm chữ, không free; `font2` mở 2 lần | `preGameplay.c` | Thấp |

Bug khác: `processEvents()` thiếu `return` (UB), `while (&quit)` lặp vô hạn, `clean()` truyền mảng vào `SDL_DestroyTexture`, header guard `map.h` sai + mảng `static` trong header, `dt` frame đầu rất lớn, `onTrapHit()` gọi `SDL_RenderPresent` giữa frame, `drawBackground()` gọi `SDL_SetRenderTarget` vô hiệu, repo commit `.vs/` (689MB) / `x64/` / `Debug/`.

## 2. Tóm tắt hiểu biết

- **Làm gì:** sửa toàn bộ leak; tái cấu trúc sang 1 window + 1 renderer + vòng lặp scene, tài nguyên có chủ sở hữu rõ ràng.
- **Vì sao:** RAM bùng nổ; `menu.c` 2.300 dòng copy-paste, điều hướng đệ quy.
- **Cho ai:** nhóm phát triển game.
- **Ràng buộc:** C, Visual Studio 2022, `Jungle Run.vcxproj` x64; code vào `src/` chia module; giữ gameplay/UI/luồng màn hình, chỉ sửa bug rõ ràng.
- **Bỏ:** login/register, `checkAccount.*`, `user.txt`; build artifacts khỏi git.
- **Non-goals:** tính năng mới, đổi UI/UX, CMake, viết lại lịch sử git, multiplayer, bảo mật tài khoản.

## 3. Giả định

1. Nút Exit / ESC ở menu chính / nút X cửa sổ đều **thoát game**.
2. Game mở thẳng vào menu chính 960×668; vào gameplay đổi 1200×800, ra lại 960×668, căn giữa mỗi lần đổi.
3. `.dll` ở root và `Resource/` giữ nguyên vị trí (working dir = root).
4. Renderer bật vsync cho mọi scene.
5. Độ khó chọn ở Game Mode chỉ sống trong bộ nhớ khi chạy.
6. Bộ đếm Debug chỉ biên dịch khi `_DEBUG`.
7. Game không được chạy trên máy phát triển; chỉ build. Kiểm thử runtime do người dùng làm trên máy khác.

## 4. Nhật ký quyết định

| # | Quyết định | Phương án khác | Lý do |
|---|-----------|----------------|-------|
| 1 | Sửa leak + tái cấu trúc vừa | Chỉ vá leak; viết lại toàn bộ | Leak do kiến trúc (đệ quy, init lại) — vá sẽ tái phát; viết lại dễ vỡ gameplay |
| 2 | `src/` chia module, giữ `.vcxproj` | Giữ phẳng; thêm CMake | Dễ tìm code, không đổi công cụ |
| 3 | Giữ gameplay/UI, sửa bug rõ ràng | Giữ 100%; cải tiến UI | Một số bug gắn với kiến trúc cũ |
| 4 | Bỏ login/register | Giữ format; hash mật khẩu | Người dùng chọn; −~1.500 dòng, hết vấn đề mật khẩu plaintext |
| 5 | `.gitignore` + `git rm --cached` | Không đụng; rewrite history | An toàn, đảo ngược được |
| 6 | Bộ đếm tài nguyên ở Debug | Đo RAM tay; CRT leak dump | Người dùng chọn; không chạy được trên máy dev |
| 7 | Được build, không chạy | Chỉ sửa code | Bắt lỗi biên dịch sớm |
| 8 | Kiến trúc A: `SceneId xxx_run(App*)` + `switch` trong main | B: state machine con trỏ hàm; C: vá tại chỗ | Ít code nhất, giữ hình dạng vòng lặp hiện có của menu → ít rủi ro lệch UI; B là trừu tượng chưa cần |
| 9 | Bộ đếm hiển thị trên tiêu đề cửa sổ + log khi đổi scene | Overlay text trong game | Không tạo texture → bộ đếm không tự leak; chạy ở mọi scene |
| 10 | `core/text.c` + `Label` → `res_text()` + `hud_label()` static trong render.c | File text riêng | Ít file hơn, cùng tác dụng |
| 11 | Màn menu/story/help/donate/gamemode vẽ lại toàn bộ mỗi frame; intro giữ vẽ cộng dồn | Port nguyên cách vẽ chắp vá | Code ngắn hơn nhiều, không dựa vào back-buffer sau `RenderPresent` |
| 12 | Mũi tên Game Mode dùng 60pt cả khi hover | Giữ đổi sang 40pt kéo giãn | Bản cũ không nhất quán |
| 13 | `GameState` giữ con trỏ `renderer` (mượn) | Truyền renderer vào mọi hàm vẽ | Ít thay đổi chữ ký |
| 14 | Gộp GĐ2+3 của thiết kế | Giữ tách | Game và scene phải đổi cùng lúc mới build được |
| 15 | Sửa thêm: tràn `lineTexture[2]`, đọc `story[7]`, R/X bị nuốt, Game Mode reset độ khó, `drawTilesMap` copy `Map` | — | Phát hiện khi lập kế hoạch |
| 16 | Trúng bẫy: nhấp nháy đỏ mỗi 100ms khi bất tử | `RenderPresent` giữa frame như cũ | Hiệu ứng giữ được, không nháy cả màn hình |

## 5. Thiết kế

### 5.1 Bố cục

```
src/
  main.c              app_init → vòng lặp scene → app_quit
  core/
    app.h/.c          App { window, renderer, difficulty, modeLabel, clickSound, menuMusic }
                      app_init(): SDL/IMG/TTF/Mixer init + Mix_OpenAudio đúng 1 lần
                      app_resize(app, w, h)
    res.h/.c          res_texture_bmp/img, res_destroy_texture, res_font, res_close_font,
                      res_wav, res_free_wav, res_mus, res_free_mus, res_debug_title
    text.h/.c         text_texture(); Label { SDL_Texture* tex; int lastValue; }
  scenes/
    scene.h           enum SceneId { SCENE_QUIT, SCENE_MENU, SCENE_STORY, SCENE_HELP,
                                     SCENE_DONATE, SCENE_GAMEMODE, SCENE_INTRO, SCENE_GAMEPLAY }
    menu.c story.c help.c donate.c gamemode.c intro.c gameplay.c   — mỗi file 1 hàm public xxx_run(App*)
  game/
    gamestate.h       GameState (gần như struct cũ)
    assets.c          game_assets_load / game_assets_free / game_reset
    physics.c collision.c map.c render.c
    levels.h/.c       lv0..lv15: extern trong .h, định nghĩa trong .c
```

Xóa: `checkAccount.*`, `user.txt`, login/register trong `menu.c`, `test.obj`/`SDL_1.obj` cũ.

### 5.2 Vòng lặp scene

```c
SceneId s = SCENE_MENU;
while (s != SCENE_QUIT) {
    switch (s) {
    case SCENE_MENU:     s = menu_run(&app);     break;
    case SCENE_GAMEPLAY: s = gameplay_run(&app); break;
    /* ... */
    }
}
```

Không hàm nào gọi scene khác. Mẫu mỗi scene — một điểm thoát:

```c
SceneId help_run(App* app) {
    SceneId next = SCENE_HELP;
    /* load */
    while (next == SCENE_HELP) { /* event → next; render */ }
    /* free toàn bộ */
    return next;
}
```

Luồng: `MENU ↔ STORY/HELP/DONATE/GAMEMODE`, `MENU → INTRO → GAMEPLAY → MENU`. `SDL_QUIT` ở bất kỳ đâu → `SCENE_QUIT`.

### 5.3 Vòng đời tài nguyên

| Tầng | Sống | Gồm |
|------|------|-----|
| App | cả chương trình | window, renderer (`ACCELERATED \| PRESENTVSYNC`), `click.wav`, `MainMenu2.mp3` |
| Scene | vào → return của `xxx_run` | texture/font/nhạc riêng màn |
| Ván chơi | — | `game_reset()` chỉ ghi lại state, không nạp gì |

Sửa leak cụ thể:
- **#1:** font gameplay mở 1 lần trong `game_assets_load`; `render_status` dùng `Label` — chỉ render lại khi `lives`/`health_potion_counter` đổi (destroy texture cũ trước).
- **#1b:** `Over()` dùng font + texture tiêu đề và dòng hướng dẫn tạo 1 lần khi vào trạng thái kết thúc, free khi rời.
- **#2:** `load_game` → `game_assets_load` (1 lần khi vào gameplay) + `game_reset` (mỗi ván, mỗi Retry).
- **#3:** điều hướng bằng `return SceneId`.
- **#4:** hover dùng 2 texture (thường/đỏ) tạo sẵn khi vào scene, chỉ chọn cái để vẽ.
- **#5:** `UpdateText` destroy texture cũ trước khi tạo mới; `font2` mở 1 lần.

### 5.4 Bug rõ ràng sẽ sửa

- `processEvents()` thêm `return 0`.
- `lastFrameTime` khởi tạo = thời điểm bắt đầu.
- `onTrapHit()`: chỉ set color mod đỏ và vẽ 1 lần, bỏ `SDL_RenderPresent`/`SDL_SetRenderTarget`.
- `drawBackground()`: bỏ `SDL_SetRenderTarget` vô hiệu (hình không đổi).
- Header guard, mảng level ra `.c`.
- `clean()` → `game_assets_free()` destroy từng phần tử.
- `while (&quit)` → vòng lặp scene chuẩn.

### 5.5 Xử lý lỗi

- `res_*`: `SDL_Log("load fail: %s: %s", path, SDL_GetError())`, trả `NULL`.
- Thiếu tài nguyên bắt buộc (window, renderer, font, texture gameplay) → `SDL_ShowSimpleMessageBox` + trả `SCENE_QUIT` (đi qua dọn dẹp, thay cho `exit(1)` im lặng).
- Thiếu âm thanh → bỏ qua, chạy không tiếng (như hiện tại).

### 5.6 Bộ đếm Debug

- `res.c` giữ `static int live[TEXTURE, FONT, CHUNK, MUSIC]`, tăng/giảm trong `res_*`, chỉ khi `_DEBUG`.
- Mỗi giây cập nhật tiêu đề: `Jungle Run [tex 42 font 3 wav 7 mus 2 | sdl_alloc 1234]` (`SDL_GetNumAllocations()` bắt cả surface quên free).
- Mỗi lần đổi scene: `SDL_Log("[scene MENU->HELP] tex=.. font=.. wav=.. mus=.. alloc=..")` ra console.

## 6. Kiểm thử

**Máy dev (build only):** MSBuild `Debug|x64` và `Release|x64` sau mỗi giai đoạn — 0 lỗi, không thêm warning.

**Checklist runtime (người dùng, bản Debug):**

1. Ở menu, ghi lại con số trên tiêu đề làm mốc. Rê chuột qua lại 30 giây → không đổi.
2. Vào/ra mỗi màn phụ (Story, Help, Donate, Game Mode) 5 lần → về menu bằng mốc.
3. Chơi liên tục 15 phút → `tex/font/wav/mus` đứng yên; RAM Task Manager phẳng sau 30s đầu.
4. Chết → đứng ở màn kết thúc 1 phút → R 20 lần → con số không tăng.
5. Bấm X về menu → bằng mốc.
6. Kiểm tra trực quan: menu, hover, story, help, donate, game mode, intro chữ chạy, gameplay, 3 kết thúc — giống bản cũ.

## 7. Triển khai

| GĐ | Nội dung | Xong khi |
|----|----------|----------|
| 0 | `.gitignore`, `git rm --cached .vs x64 Debug` | `git status` sạch artifacts |
| 1 | `src/`, `core/` (app, res, text), vòng lặp scene, cập nhật `.vcxproj`, xóa login/register/checkAccount | Build xanh |
| 2 | menu, story, help, donate, gamemode → `return SceneId`, sửa leak hover | Build xanh |
| 3 | intro + gameplay: assets/reset, `Label`, `Over()`, `levels.c`, bug mục 5.4 | Build xanh |
| 4 | Build Debug + Release, rà mọi cặp tạo/hủy tài nguyên | 0 lỗi, checklist sẵn sàng |

## 8. Rủi ro

- Port ~800 dòng menu với vòng lặp lồng → có thể lệch vị trí/thứ tự vẽ; chỉ phát hiện khi chạy (checklist mục 6).
- Đường dẫn include/lib SDL trong `.vcxproj` có thể phụ thuộc máy.
- Không chạy được trên máy dev → lỗi runtime (crash, sai logic) chỉ lộ ở bước kiểm thử của người dùng.
