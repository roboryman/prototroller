set(PICO_BOARD_HEADER_DIRS ${CMAKE_SOURCE_DIR}) # Custom master board config

cmake_minimum_required(VERSION 3.13.0)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(module C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)

add_subdirectory(host)
add_subdirectory(master)
add_subdirectory(module)