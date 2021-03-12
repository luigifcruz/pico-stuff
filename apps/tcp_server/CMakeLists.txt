cmake_minimum_required(VERSION 3.12)

project(pico-tcp-server)

add_executable(tcp_server main.c)

target_link_libraries(tcp_server LINK_PUBLIC
    usb_network_stack
    hardware_adc
)

pico_add_extra_outputs(tcp_server)

pico_enable_stdio_usb(tcp_server 1)
pico_enable_stdio_uart(tcp_server 0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)