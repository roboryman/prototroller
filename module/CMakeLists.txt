add_executable(button-module)
add_executable(joystick-module)

target_sources(button-module PRIVATE
    libraries/Component.cpp
    libraries/SPISlave.cpp
    src/button.cpp
)

target_sources(joystick-module PRIVATE
    libraries/Component.cpp
    libraries/SPISlave.cpp
    src/joystick.cpp
)

target_link_libraries(button-module PRIVATE
    pico_stdlib 
    pico_binary_info
    hardware_gpio
    hardware_spi
)

target_link_libraries(joystick-module PRIVATE
    pico_stdlib 
    pico_binary_info
    hardware_gpio
    hardware_spi
    hardware_adc
)

pico_enable_stdio_usb(button-module 1)
pico_enable_stdio_usb(joystick-module 1)

pico_add_extra_outputs(button-module)
pico_add_extra_outputs(joystick-module)



