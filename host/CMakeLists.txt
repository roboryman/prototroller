# add_executable(tinyusb-example)

# pico_enable_stdio_usb(tinyusb-example 1)

# add_compile_options(-Wall
#         -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
#         -Wno-unused-function # we have some for the docs that aren't called
#         -Wno-maybe-uninitialized
# )
# # pico_generate_pio_header(spi-master
# #     ${CMAKE_CURRENT_LIST_DIR}/libraries/ws2812.pio
# # )

# target_sources(tinyusb-example PRIVATE
#     #sources here
# )

# target_link_libraries(tinyusb-example PRIVATE
#     pico_stdlib 
#     hardware_gpio
#     hardware_clocks
#     hardware_pio
#     hardware_spi
#     hardware_i2c
# )

# pico_add_extra_outputs(tinyusb-example)