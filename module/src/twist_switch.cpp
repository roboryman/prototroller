#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "../libraries/SPISlave.h"
#include "../../prototroller.h"

#define moduleID TWIST_SWITCH

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

    // Initialize any needed GPIO
    // gpio_init(MODULE_???_PIN);
    // gpio_set_dir(MODULE_???_PIN1, GPIO_IN);
    // gpio_set_pulls(MODULE_???_PIN1, false, false);

    // Wait for identification
    spi.SlaveWrite(out_buf, in_buf, BUF_LEN);

    // After identifier is sent, continually send the GPIO state
    while(true)
    {
        // Get input
        //bool button1_state = gpio_get(MODULE_???_PIN1);

        // Load into the output buffer
        //out_buf[0] = button1_state;

        // Sync with the master
        spi.SlaveWrite(out_buf, in_buf, BUF_LEN);
    }

}