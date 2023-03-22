#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "../libraries/SPISlave.h"
#include "../../prototroller.h"

#define moduleID BUTTON_MOMENTARY

SPISlave spi(
    spi_default,
    MODULE_SPI_TX_PIN,
    MODULE_SPI_RX_PIN,
    MODULE_SPI_SCK_PIN,
    MODULE_SPI_CSN_PIN,
    moduleID
);

// Declare and initialize buffers
uint8_t out_buf[BUF_LEN] = {0};
uint8_t in_buf[BUF_LEN] = {0};

int main() {
    // Initialize SPI as a slave
    spi.SlaveInit();

    // Initialize the active-low button GPIO
    gpio_init(MODULE_BUTTON_PIN);
    gpio_set_dir(MODULE_BUTTON_PIN, GPIO_IN);
    gpio_set_pulls(MODULE_BUTTON_PIN, false, false);

    // Wait for identification
    spi.SlaveReadWrite(out_buf, in_buf, BUF_LEN);

    // After identifier is sent, continually send the GPIO state
    while(true)
    {
        // Get the active-low button state
        bool button_state = gpio_get(MODULE_BUTTON_PIN);

        // Set the first byte to the button state
        out_buf[0] = button_state;

        // DEBUG - Set ALL buffer data to the button state
        // for(uint16_t i = 0; i < BUF_LEN; i++)
        // {
        //     out_buf[i] = button_state;
        // }

        spi.SlaveReadWrite(out_buf, in_buf, BUF_LEN);
    }

}