cmake_minimum_required(VERSION 3.12)

add_library(fusb fusb.h)

target_link_libraries(fusb
    pico_stdlib
    pico_stdio
    hardware_i2c
)

target_include_directories(fusb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
