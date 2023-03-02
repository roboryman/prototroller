set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
add_executable(master)
add_executable(master-debug)

pico_enable_stdio_usb(master 1)
pico_enable_stdio_usb(master-debug 1)

target_sources(master PRIVATE
    libraries/SPIMaster.cpp
    src/master.cpp
    src/usb_descriptors.c
)

target_sources(master-debug PRIVATE
    libraries/SPIMaster.cpp
    src/master-debug.cpp
    src/usb_descriptors.c
)

target_include_directories(master PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_include_directories(master-debug PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(master PRIVATE
    pico_stdlib 
    hardware_gpio
    hardware_clocks
    hardware_pio
    hardware_spi
    hardware_i2c
    tinyusb_device
    tinyusb_board
)

target_link_libraries(master-debug PRIVATE
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
pico_add_extra_outputs(master-debug)