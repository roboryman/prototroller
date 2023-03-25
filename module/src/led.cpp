#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "../libraries/SPISlave.h"
#include "../../prototroller.h"
#include "../../commons.h"

#define moduleID LED

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

    // Initialize the active-high LED GPIO
    gpio_init(MODULE_LED_PIN);
    gpio_set_dir(MODULE_LED_PIN, GPIO_OUT);
    gpio_set_pulls(MODULE_LED_PIN, false, false);
    gpio_put(MODULE_LED_PIN, 0);

    // Wait for identification
    spi.SlaveReadWrite(out_buf, in_buf, BUF_LEN);

    while(true)
    {
        // Read data in from the master
        spi.SlaveReadWrite(out_buf, in_buf, BUF_LEN);

        // Place the LED output on the appropraite pin
        gpio_put(MODULE_LED_PIN, in_buf[0]);
    }

}