cmake_minimum_required(VERSION 3.12)

add_library(bmp390 bmp390.h)

target_link_libraries(bmp390
    pico_stdlib
    pico_stdio
    hardware_i2c
)

target_include_directories(bmp390 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
