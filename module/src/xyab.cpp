#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "../libraries/SPISlave.h"
#include "../../prototroller.h"

#define moduleID XYAB

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

    // Initialize the active-low GPIOs
    gpio_init(MODULE_XYAB_PIN1);
    gpio_init(MODULE_XYAB_PIN2);
    gpio_init(MODULE_XYAB_PIN3);
    gpio_init(MODULE_XYAB_PIN4);
    gpio_set_dir(MODULE_XYAB_PIN1, GPIO_IN);
    gpio_set_dir(MODULE_XYAB_PIN2, GPIO_IN);
    gpio_set_dir(MODULE_XYAB_PIN3, GPIO_IN);
    gpio_set_dir(MODULE_XYAB_PIN4, GPIO_IN);
    gpio_set_pulls(MODULE_XYAB_PIN1, false, false);
    gpio_set_pulls(MODULE_XYAB_PIN2, false, false);
    gpio_set_pulls(MODULE_XYAB_PIN3, false, false);
    gpio_set_pulls(MODULE_XYAB_PIN4, false, false);

    // Wait for identification
    spi.SlaveWrite(out_buf, in_buf, BUF_LEN);

    // After identifier is sent, continually send the GPIO state
    while(true)
    {
        // Get the active-low button state
        bool button1_state = gpio_get(MODULE_XYAB_PIN1);
        bool button2_state = gpio_get(MODULE_XYAB_PIN2);
        bool button3_state = gpio_get(MODULE_XYAB_PIN3);
        bool button4_state = gpio_get(MODULE_XYAB_PIN4);

        // Load the button states into the output buffer
        out_buf[0] = button1_state;
        out_buf[1] = button2_state;
        out_buf[2] = button3_state;
        out_buf[3] = button4_state;

        spi.SlaveWrite(out_buf, in_buf, BUF_LEN);
    }

}