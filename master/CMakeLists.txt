add_executable(spi-master)

pico_enable_stdio_usb(spi-master 1)

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