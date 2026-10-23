# CI: build .exe khi merge vào main + nút tải trên README

Trạng thái: đã triển khai trên `ci/release-exe` (tách từ `refactor/cpp-oop`), chưa push.

## 1. Tóm tắt yêu cầu
- Workflow GitHub Actions `.github/workflows/release.yml`.
- Push/merge vào `main` → build Release x64 → `JungleRun.zip` (exe + 8 DLL + `Resource/`) → GitHub Release mới `build-<run_number>`, release notes tự sinh.
- PR vào `main` và chạy tay (`workflow_dispatch`): chỉ build + artifact (7 ngày), không release.
- README có nút tải cố định `releases/latest/download/JungleRun.zip` — không có bot commit vào `main`.
- Release|x64 đổi sang `/SUBSYSTEM:WINDOWS` (không hiện console); Debug giữ console cho bộ đếm leak.
- Không làm: ký số, installer, bản x86/Debug, test runtime tự động.

## 2. Giả định
1. Runner `windows-2022` có VS 2022 + v143 (đúng toolset project khai báo) → không truyền `PlatformToolset`. Ghim `windows-2022` thay vì `windows-latest`.
2. SDL lấy từ `External library/` trong repo; 8 DLL ở gốc repo giống từng byte bản x64 trong `External library/`.
3. Zip ~100 MB, build ~3–5 phút. Repo public → ai cũng tải được.
4. Chỉ dùng `GITHUB_TOKEN`. PR từ fork luôn nhận token chỉ-đọc.
5. Build lỗi → không release; README vẫn trỏ bản thành công gần nhất. `concurrency` xếp hàng các lần chạy trên `main`.

## 3. Decision log
| # | Quyết định | Phương án khác | Lý do |
|---|---|---|---|
| 1 | Zip chạy ngay (exe + DLL + Resource) | Chỉ exe; cả hai | Exe đơn lẻ không chạy được |
| 2 | GitHub Release + link `latest/download` cố định | Bot sửa README mỗi build; artifact Actions | Không commit bot, không vòng lặp; artifact cần đăng nhập và hết hạn |
| 3 | Release mới mỗi lần merge, tag `build-N` | Một release ghi đè; chỉ khi tag `v*` | Giữ lịch sử, tải lại bản cũ được |
| 4 | Release|x64 dùng SubSystem Windows | Giữ Console | Người chơi không thấy cửa sổ console; SDL2main có sẵn WinMain |
| 5 | Thêm build PR + `workflow_dispatch` (không release) | Chỉ push main | Bắt lỗi build trước khi merge |
| 6 | Branch `ci/release-exe` từ `refactor/cpp-oop` | Commit thẳng lên branch refactor; branch từ main | Bản đầu tiên chỉ ra khi user merge sau checklist runtime |
| 7 | Một job, bước release có `if:` (phương án A) | Hai job tách quyền (B); `softprops/action-gh-release` (C) | Gọn nhất, `gh` có sẵn trên runner, không thêm action bên thứ ba |
| 8 | Giữ đủ `Resource/` | Loại trừ file không dùng (~35 MB) | Không rủi ro thiếu file; dọn file thừa là việc riêng trong repo |

## 4. Thiết kế
- Bước: checkout → setup-msbuild → `msbuild "Jungle Run.vcxproj" -p:Configuration=Release -p:Platform=x64 -m` → Package (pwsh: chép, kiểm 8 DLL + `Resource/Levels/course.txt`, `7z a -tzip`) → upload-artifact → `gh release create build-N dist/JungleRun.zip --target <sha> --generate-notes` (chỉ khi push vào `main`).
- Zip có thư mục gốc `JungleRun/`.

## 5. Kiểm chứng
Trên máy dev (đã làm): Release x64 build 0 lỗi / 0 warning (v145); `dumpbin` báo `subsystem (Windows GUI)`; YAML parse được; chạy thử phần chép + kiểm của bước Package (7z không có trên máy dev); mọi đường dẫn tài nguyên trong `src/` đều có trong git.

Cần user làm: push `ci/release-exe`, mở PR vào `main` → Actions xanh, có artifact `JungleRun`; tải artifact, chạy thử. Sau merge → có release `build-N`, nút README tải được.

## 6. Rủi ro
- `windows-2022` sẽ bị GitHub gỡ: đổi runner + thêm `-p:PlatformToolset=v145`.
- Đổi tên file workflow → `run_number` đếm lại → tag `build-N` trùng, bước release lỗi.
- Bản release đầu tiên là bản C++ chưa chạy runtime → chỉ merge sau checklist.
