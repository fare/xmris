cmake_minimum_required(VERSION 3.22)
project(xmris LANGUAGES C CXX OBJC)
include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "")
set(SDL_STATIC ON CACHE BOOL "")
set(SDL_TEST_LIBRARY OFF CACHE BOOL "")
FetchContent_Declare(SDL3
  URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.20/SDL3-3.4.20.tar.gz
  URL_HASH SHA256=f3bb06f55bed9557227a700af2ea62e49497ebdf58037732a731a5fb3ef8581f)
FetchContent_MakeAvailable(SDL3)

set(root "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(sources options.c platform-sdl.c video-sdl.c apple.c defmris.c demo.c draw.c
    monster.c move.c player.c scoring.c timer.c sound.c xmris.c)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  list(APPEND sources android/input.c)
endif()
list(TRANSFORM sources PREPEND "${root}/")
add_executable(xmris MACOSX_BUNDLE ${sources})
target_include_directories(xmris PRIVATE "${root}")
target_compile_definitions(xmris PRIVATE XMRIS_SDL)
target_link_libraries(xmris PRIVATE SDL3::SDL3-static)
set_target_properties(xmris PROPERTIES C_STANDARD 11
  MACOSX_BUNDLE_BUNDLE_NAME "Mr Is"
  MACOSX_BUNDLE_GUI_IDENTIFIER org.xmris.game
  MACOSX_BUNDLE_SHORT_VERSION_STRING 4.1.2 MACOSX_BUNDLE_BUNDLE_VERSION 2)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set_target_properties(xmris PROPERTIES MACOSX_BUNDLE_INFO_PLIST
    "${CMAKE_CURRENT_SOURCE_DIR}/iOS.plist.in" XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
endif()
