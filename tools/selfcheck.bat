@echo off
rem Builds and runs tools\selfcheck.exe: checks worlds, courses and tuning.txt with the game's loaders.
rem No game window is opened. Works from any folder; needs Visual Studio (Build Tools) with C++.
setlocal
cd /d "%~dp0.."
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath`) do set "VSDIR=%%i"
call "%VSDIR%\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
if not exist x64 mkdir x64
cl /nologo /std:c++17 /W4 /WX /EHsc /Isrc /I"External library\SDL2\include" tools\selfcheck.cpp src\game\LevelLoader.cpp src\game\Tuning.cpp /Fe:x64\selfcheck.exe /Fo:x64\ /link /LIBPATH:"External library\SDL2\lib\x64" SDL2.lib /SUBSYSTEM:CONSOLE || exit /b 1
x64\selfcheck.exe
