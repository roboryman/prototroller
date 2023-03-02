#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "../libraries/Component.h"
#include "../libraries/SPISlave.h"
#include "../../prototroller.h"

Component button;
SPISlave spi(
    spi_default,
    MODULE_SPI_TX_PIN,
    MODULE_SPI_RX_PIN,
    MODULE_SPI_SCK_PIN,
    MODULE_SPI_CSN_PIN,
    BUTTON_IDENTITY
);

// Declare and initialize buffers
uint8_t out_buf[BUF_LEN] = {0};
uint8_t in_buf[BUF_LEN] = {0};

int main() {
    //board_init();

    //stdio_init_all();

    printf("BUTTON MODULE\n");

    // Emable SPI0 at 1 MHz and connect to GPIOs
    spi.SlaveInit();

    printf("Slave initialized.\n");

    // Initialize the active-low button GPIO
    gpio_init(MODULE_BUTTON_PIN);
    gpio_set_dir(MODULE_BUTTON_PIN, GPIO_IN);
    gpio_set_pulls(MODULE_BUTTON_PIN, false, false);

    printf("Button GPIO initialized.\n");

    printf("Wait for ID.\n");
    spi.SlaveWrite(out_buf, in_buf, BUF_LEN);

    // After identifier is sent, continually send the GPIO state
    while(true)
    {
        bool button_state = gpio_get(MODULE_BUTTON_PIN); // Active-Low
        out_buf[0] = button_state;

        // DEBUG - Set ALL buffer data to the button state
        for(uint16_t i = 0; i < BUF_LEN; i++)
        {
            out_buf[i] = button_state;
        }

        //printf( button_state ? "Not Pressed\n" : "Pressed\n");

        spi.SlaveWrite(out_buf, in_buf, BUF_LEN);

        // if(spi.SlaveWrite(out_buf, in_buf, BUF_LEN)) {
        //     printf("Slave Write Executed\n");
        // }
        // else {
        //     printf("Slave Write FAILED\n");
        // }
    }

}