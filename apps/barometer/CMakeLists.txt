cmake_minimum_required(VERSION 3.12)

project(pico-barometer)

add_executable(barometer test.c)

target_link_libraries(barometer LINK_PUBLIC bmp390)

pico_add_extra_outputs(barometer)

pico_enable_stdio_usb(barometer 1)
pico_enable_stdio_uart(barometer 0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)