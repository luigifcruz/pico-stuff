cmake_minimum_required(VERSION 3.12)

project(adc-dma-chain)

add_executable(adc_dma_chain
    main.c
)

target_link_libraries(adc_dma_chain
    pico_stdlib
    pico_stdio
    hardware_adc
    hardware_dma
    hardware_irq
)

target_include_directories(adc_dma_chain PRIVATE .)

pico_enable_stdio_usb(adc_dma_chain 1)
pico_enable_stdio_uart(adc_dma_chain 0)

pico_add_extra_outputs(adc_dma_chain)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
