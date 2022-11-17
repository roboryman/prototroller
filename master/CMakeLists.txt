cmake_minimum_required(VERSION 3.13)

add_executable(main)

message(${CMAKE_CURRENT_LIST_DIR})

target_sources(main PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
        )
        
        # Make sure TinyUSB can find tusb_config.h
        target_include_directories(main PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src
        )

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(main PUBLIC
        pico_stdlib
        tinyusb_device
        tinyusb_board
        )

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions(dev_hid_composite PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs(main)