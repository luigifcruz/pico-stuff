cmake_minimum_required(VERSION 3.12)

project(pico-bmp180)

add_executable(bmp180
    test.c
)

target_link_libraries(bmp180
    pico_stdlib
    pico_stdio
    hardware_i2c
)

pico_enable_stdio_usb(bmp180 1)
pico_enable_stdio_uart(bmp180 0)

pico_add_extra_outputs(bmp180)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
