cmake_minimum_required(VERSION 3.12)

project(pico-tcp-server)

add_executable(tcp_server
    usb_descriptors.c
    main.c
)

target_link_libraries(tcp_server
    pico_stdlib
    pico_stdio
    tinyusb_host
    tinyusb_board
    tinyusb_net
    hardware_i2c
    lwip
)

target_include_directories(tcp_server PRIVATE
    .
    ${CMAKE_SOURCE_DIR}/lib/bmp180
)

pico_enable_stdio_usb(tcp_server 1)
pico_enable_stdio_uart(tcp_server 0)

pico_add_extra_outputs(tcp_server)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
