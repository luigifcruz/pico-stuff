cmake_minimum_required(VERSION 3.12)

project(pico-altimeter)

add_executable(altimeter test.c)

target_link_libraries(altimeter LINK_PUBLIC littlefs bmp390)

pico_add_extra_outputs(altimeter)

pico_enable_stdio_usb(altimeter 1)
pico_enable_stdio_uart(altimeter 0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)