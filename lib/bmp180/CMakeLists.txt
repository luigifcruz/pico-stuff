cmake_minimum_required(VERSION 3.12)

add_library(bmp180 bmp180.h)

target_link_libraries(bmp180
    pico_stdlib
    pico_stdio
    hardware_i2c
)

target_include_directories (bmp180 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})