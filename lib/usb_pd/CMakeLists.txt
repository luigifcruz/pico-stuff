cmake_minimum_required(VERSION 3.12)

add_library(usbpd usb_pd.h)

target_link_libraries(usbpd
    pico_stdlib
    pico_stdio
)

target_include_directories(usbpd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
