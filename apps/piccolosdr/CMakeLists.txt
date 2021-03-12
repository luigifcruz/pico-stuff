cmake_minimum_required(VERSION 3.12)

project(piccolosdr)

add_executable(piccolosdr
    usb_descriptors.c
    main.c
)

target_link_libraries(piccolosdr
    pico_stdlib
    pico_stdio
    tinyusb_host
    tinyusb_board
    tinyusb_net
    hardware_adc
    hardware_dma
    hardware_irq
    lwip
)

target_include_directories(piccolosdr PRIVATE .)

pico_enable_stdio_usb(piccolosdr 1)
pico_enable_stdio_uart(piccolosdr 0)

pico_add_extra_outputs(piccolosdr)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
