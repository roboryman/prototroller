add_executable(master)

pico_enable_stdio_usb(master 1)

target_sources(master PRIVATE
    libraries/SPIMaster.cpp
    src/master.cpp
)

target_link_libraries(master PRIVATE
    pico_stdlib 
    hardware_gpio
    hardware_clocks
    hardware_pio
    hardware_spi
    hardware_i2c
)

pico_add_extra_outputs(master)