cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_spimaster C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()


# Initialize the SDK
pico_sdk_init()

add_executable(spi-master)

pico_enable_stdio_usb(spi-master 1)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)
# pico_generate_pio_header(spi-master
#     ${CMAKE_CURRENT_LIST_DIR}/libraries/ws2812.pio
# )

target_sources(spi-master PRIVATE
    libraries/SPIMaster.cpp
    src/spi-master.cpp
)

target_link_libraries(spi-master PRIVATE
    pico_stdlib 
    hardware_gpio
    hardware_clocks
    hardware_pio
    hardware_spi
    hardware_i2c
)

pico_add_extra_outputs(spi-master)