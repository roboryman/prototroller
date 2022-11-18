#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "../libraries/Component.h"
#include "../libraries/SPISlave.h"

#define BUTTON_PIN 15

int main() {
    Component button;
    SPISlave spi(
        spi_default,
        SPI_TX_PIN,
        SPI_RX_PIN,
        SPI_SCK_PIN,
        SPI_CSN_PIN
    );

    // Initialize logging if enabled
    //if(LOGGING)
    //{
        stdio_init_all();
    //}

    sleep_ms(10000);

    printf("BUTTON MODULE\n");

    // Emable SPI0 at 1 MHz and connect to GPIOs
    spi.SlaveInit();

    printf("Slave initialized.\n");

    // Initialize the active-low button GPIO
    gpio_init(BUTTON_PIN);

    // Button is an input
    gpio_set_dir(BUTTON_PIN, GPIO_IN);

    // Using an external pull-up, so disable internal pulls
    gpio_set_pulls(BUTTON_PIN, false, false);

    printf("Button GPIO initialized.\n");

    // Make SPI pins available for use with PicoTool
    // bi_decl(
    //     bi_4pins_with_func(
    //         SPI_MOSI_PIN,
    //         SPI_MISO_PIN,
    //         SPI_SCK_PIN,
    //         SPI_SS_PIN,
    //         GPIO_FUNC_SPI
    //     )
    // );

    // First data sent over SPI is the module identifier
    //spi.SlaveWriteIdentifier(BUTTON_MODULE_ID);

    // Declare button state data holder
    //uint8_t state;

    // Declare and initialize buffers
    uint8_t out_buf[BUF_LEN] = {0};
    uint8_t in_buf[BUF_LEN] = {0};

    printf("Module buffers initialized.\n");

    // After identifier is sent, continually send the GPIO state
    while(true)
    {
        bool button_state = gpio_get(BUTTON_PIN); // Active-Low
        out_buf[0] = button_state;

        // DEBUG
        for(uint16_t i = 0; i < BUF_LEN; i++)
        {
            out_buf[i] = button_state;
        }

        printf("(DEBUG) Outbut buffer set to button state\n");

        printf( button_state ? "Not Pressed\n" : "Pressed\n");

        if(spi.SlaveWrite(out_buf, in_buf, BUF_LEN)) {
            printf("Slave Write Executed\n");
        }
        else {
            printf("Elon Musk is our master\n");
        }
    

        // Write the button state, and re-send module identifier if requested
        // while(spi.SlaveWrite(&state, 1))
        // {
        //     spi.SlaveWriteIdentifier(BUTTON_MODULE_ID);
        // }
    }

}