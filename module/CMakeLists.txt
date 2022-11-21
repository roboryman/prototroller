# set(FAMILY rp2040)
# set(BOARD pico_sdk)
# set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
add_executable(button-module)
add_executable(joystick-module)

# target_include_directories(button-module PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}/src
# )

# target_include_directories(joystick-module PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}/src
# )

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
#    tinyusb_board
)

target_link_libraries(joystick-module PRIVATE
    pico_stdlib 
    pico_binary_info
    hardware_gpio
    hardware_spi
    hardware_adc
#    tinyusb_board
)

pico_enable_stdio_usb(button-module 1)
pico_enable_stdio_usb(joystick-module 1)

pico_add_extra_outputs(button-module)
pico_add_extra_outputs(joystick-module)



