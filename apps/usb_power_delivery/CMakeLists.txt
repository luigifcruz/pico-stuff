cmake_minimum_required(VERSION 3.12)

project(pico-usbpd)

add_executable(usbpd_example test.c)

target_link_libraries(usbpd_example LINK_PUBLIC fusb usbpd)

pico_add_extra_outputs(usbpd_example)

pico_enable_stdio_usb(usbpd_example 1)
pico_enable_stdio_uart(usbpd_example 0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
