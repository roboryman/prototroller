add_executable(spi-slave)
add_executable(button)
#add_executable(joystick)

target_sources(spi-slave PRIVATE
    libraries/SPISlave.cpp
    src/spi-slave.cpp
)

target_sources(button PRIVATE
    "libraries/CAL.cpp"
    "libraries/SPISlave.cpp"
    "src/button.cpp"
)

# target_sources(joystick PRIVATE
#     "libraries/CAL.cpp"
#     "libraries/SPISlave.cpp"
#     "src/joystick.cpp"
# )

target_link_libraries(spi-slave PRIVATE
    pico_stdlib 
    hardware_gpio
    hardware_clocks
    hardware_pio
    hardware_spi
    hardware_i2c
)

target_link_libraries(button PRIVATE
    pico_stdlib 
    pico_binary_info
    hardware_gpio
    hardware_spi
)

# target_link_libraries(joystick PRIVATE
#     pico_stdlib 
#     pico_binary_info
#     hardware_gpio
#     hardware_spi
# )

pico_add_extra_outputs(spi-slave)
pico_add_extra_outputs(button)
#pico_add_extra_outputs(joystick)



