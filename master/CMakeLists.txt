set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
add_executable(spi-master)

pico_enable_stdio_usb(spi-master 1)

target_sources(spi-master PRIVATE
    libraries/SPIMaster.cpp
    src/spi-master.cpp
    src/usb_descriptors.c
)

target_include_directories(spi-master PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(spi-master PRIVATE
    pico_stdlib 
    hardware_gpio
    hardware_clocks
    hardware_pio
    hardware_spi
    hardware_i2c
    tinyusb_device
    tinyusb_board
)

pico_add_extra_outputs(spi-master)