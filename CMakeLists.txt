cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(pico-stuff)

pico_sdk_init()

add_subdirectory(apps)
add_subdirectory(lib)