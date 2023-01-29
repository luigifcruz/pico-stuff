cmake_minimum_required(VERSION 3.12)

project(pico-iperf-server)

add_executable(iperf_server main.c)

target_link_libraries(iperf_server LINK_PUBLIC
    tinyusb_host
    tinyusb_board
    tinyusb_net
    lwip
    pico_lwip_iperf
    usb_network_stack
)

pico_add_extra_outputs(iperf_server)

pico_enable_stdio_usb(iperf_server 0)
pico_enable_stdio_uart(iperf_server 0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)