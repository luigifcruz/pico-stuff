cmake_minimum_required(VERSION 3.12)

project(pico-cdc_ecm_device)

add_executable(cdc_ecm_device
    usb_descriptors.c
    main.c
)

target_link_libraries(cdc_ecm_device
    pico_stdlib
    pico_stdio
    tinyusb_host
    tinyusb_board
    tinyusb_net
    lwip
)

target_include_directories(cdc_ecm_device PRIVATE .)

pico_enable_stdio_usb(cdc_ecm_device 1)
pico_enable_stdio_uart(cdc_ecm_device 0)

pico_add_extra_outputs(cdc_ecm_device)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)