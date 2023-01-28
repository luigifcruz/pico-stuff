cmake_minimum_required(VERSION 3.12)

project(pico-filesystem)

add_executable(filesystem test.c)

target_link_libraries(filesystem LINK_PUBLIC littlefs)

pico_add_extra_outputs(filesystem)

pico_enable_stdio_usb(filesystem 1)
pico_enable_stdio_uart(filesystem 0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)