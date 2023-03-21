# set(FAMILY rp2040)
# set(BOARD pico_sdk)
# set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
add_executable(button_maintained)
add_executable(button_momentary)
add_executable(xyab)
add_executable(joystick)

# target_include_directories(button-module PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}/src
# )

# target_include_directories(joystick-module PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}/src
# )

target_sources(button_maintained PRIVATE
    libraries/SPISlave.cpp
    src/button_maintained.cpp
)

target_sources(button_momentary PRIVATE
    libraries/SPISlave.cpp
    src/button_momentary.cpp
)

target_sources(xyab PRIVATE
    libraries/SPISlave.cpp
    src/xyab.cpp
)

target_sources(joystick PRIVATE
    libraries/SPISlave.cpp
    src/joystick.cpp
)

target_link_libraries(button_maintained PRIVATE
    pico_stdlib 
    pico_binary_info
    hardware_gpio
    hardware_spi
)

target_link_libraries(button_momentary PRIVATE
    pico_stdlib 
    pico_binary_info
    hardware_gpio
    hardware_spi
)

target_link_libraries(xyab PRIVATE
    pico_stdlib 
    pico_binary_info
    hardware_gpio
    hardware_spi
)

target_link_libraries(joystick PRIVATE
    pico_stdlib 
    pico_binary_info
    hardware_gpio
    hardware_spi
    hardware_adc
)

pico_enable_stdio_usb(button_maintained 1)
pico_enable_stdio_usb(button_momentary 1)
pico_enable_stdio_usb(xyab 1)
pico_enable_stdio_usb(joystick 1)

pico_add_extra_outputs(button_maintained)
pico_add_extra_outputs(button_momentary)
pico_add_extra_outputs(xyab)
pico_add_extra_outputs(joystick)



