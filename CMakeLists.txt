# Linux/macOS build (Windows uses "Jungle Run.vcxproj").
#   sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
#   cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
#   ./build/JungleRun        (run from the repo folder so Resource/ is found)
cmake_minimum_required(VERSION 3.16)
project(JungleRun CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(JungleRun ${SOURCES})
target_include_directories(JungleRun PRIVATE src)
target_link_libraries(JungleRun PRIVATE
  $<IF:$<TARGET_EXISTS:SDL2::SDL2main>,SDL2::SDL2main,>
  $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
  $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2::SDL2_image>
  $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2::SDL2_mixer>
  $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2::SDL2_ttf>)
