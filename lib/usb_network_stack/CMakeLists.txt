cmake_minimum_required(VERSION 3.12)

add_library(usb_network_stack usb_descriptors.c)

target_link_libraries(usb_network_stack
    pico_stdlib
    pico_stdio
    tinyusb_host
    tinyusb_board
    tinyusb_net
    hardware_adc
    lwip
)

target_include_directories(usb_network_stack PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})