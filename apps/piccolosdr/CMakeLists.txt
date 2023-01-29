cmake_minimum_required(VERSION 3.12)

project(piccolosdr)

add_executable(piccolosdr main.c)

target_link_libraries(piccolosdr LINK_PUBLIC
    usb_network_stack
    hardware_adc
    hardware_dma
    hardware_irq
    pico_sync
)

pico_add_extra_outputs(piccolosdr)

pico_enable_stdio_usb(piccolosdr 0)
pico_enable_stdio_uart(piccolosdr 0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
