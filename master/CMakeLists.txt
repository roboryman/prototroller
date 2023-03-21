set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
add_executable(master)

pico_enable_stdio_usb(master 1)

target_sources(master PUBLIC
    libraries/SPIMaster.cpp
    src/master.cpp
    src/usb_descriptors.c
)

target_include_directories(master PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
)

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

pico_add_extra_outputs(master)