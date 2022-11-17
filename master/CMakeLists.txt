cmake_minimum_required(VERSION 3.13)

add_executable(master)

pico_enable_stdio_usb(master 1)

target_sources(master PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/libraries/SPIMaster.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/master.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
        )
        
        # Make sure TinyUSB can find tusb_config.h
        target_include_directories(master PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src
        )

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(master PUBLIC
        pico_stdlib
        hardware_gpio
        hardware_clocks
        hardware_pio
        hardware_spi
        hardware_i2c
        tinyusb_device
        tinyusb_board
        )

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
target_compile_definitions(master PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

add_compile_options(-DCFG_TUSB_DEBUG=3)

pico_add_extra_outputs(master)