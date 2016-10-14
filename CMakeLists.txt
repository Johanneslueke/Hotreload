cmake_minimum_required(VERSION 3.6)
project(HotReload)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lSDL2 -ldl")

set(SOURCE_FILES main.cpp hotreload.h source/ConfigSetting.cpp DynamicLib.h source/DynamicLib.cpp)
add_executable(HotReload ${SOURCE_FILES} hotreload.h source/ConfigSetting.cpp)
