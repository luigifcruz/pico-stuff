cmake_minimum_required(VERSION 3.12)

project(pico-iperf-server)

add_executable(iperf_server
    usb_descriptors.c
    main.c
)

target_link_libraries(iperf_server
    pico_stdlib
    pico_stdio
    tinyusb_host
    tinyusb_board
    tinyusb_net
    lwip
)

target_include_directories(iperf_server PRIVATE .)

pico_enable_stdio_usb(iperf_server 1)
pico_enable_stdio_uart(iperf_server 0)

pico_add_extra_outputs(iperf_server)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)