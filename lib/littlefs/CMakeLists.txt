cmake_minimum_required(VERSION 3.12)

add_library(littlefs ./lfs_rp2040.c
                     ./littlefs/lfs.c
                     ./littlefs/lfs_util.c)

target_link_libraries(littlefs
    pico_stdlib
    pico_stdio
    hardware_flash
    hardware_sync
)

target_include_directories(littlefs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                           ${CMAKE_CURRENT_SOURCE_DIR}/littlefs)