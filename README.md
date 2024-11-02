# Jungle-Run-F-code
Welcome to Project "Jungle Run".
Q&A:
1/. How to set up SDL library to Microsoft Visual Studio?
https://thenumb.at/cpp-course/sdl2/01/vsSetup.html
You can see the link above but we have customized things differently. Let me guide you into the things we change:
  1. Platform: x64
  2. Change x86 -> x64.
  3. In Property Pages -> Linker -> Syste -> SubSystem -> chose Windows (/SUBSYSTEM:WINDOWS).
