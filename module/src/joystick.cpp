#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
#include "../libraries/Component.h"
#include "../libraries/SPISlave.h"

#define VRX_PIN 26
#define VRY_PIN 27

int main() {
    Component joystick;
    SPISlave spi(
        spi_default,
        SPI_TX_PIN,
        SPI_RX_PIN,
        SPI_SCK_PIN,
        SPI_CSN_PIN,
        JOYSTICK_IDENTITY
    );

    // Initialize logging if enabled
    //if(LOGGING)
    //{
        stdio_init_all();
    //}

    sleep_ms(10000);

    printf("Joystick Module");

    // Emable SPI0 at 1 MHz and connect to GPIOs
    spi.SlaveInit();

    printf("Slave initialized.\n");

    // Initialize the ADC/GPIO
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    printf("ADC initialized.\n");

    // First data sent over SPI is the module identifier
    //spi.SlaveWriteIdentifier(JOYSTICK_MODULE_ID);

    // Declare and initialize buffers
    uint8_t out_buf[BUF_LEN] = {0};
    uint8_t in_buf[BUF_LEN] = {0};

    printf("Module buffers initialized.\n");

    // After identifier is sent, continually send the GPIO state
    while(true)
    {
        // Read the X-axis by starting an ADC conversion
        adc_select_input(0); // Select ADC0 (x)
        uint16_t x = adc_read();

        // Read the Y-axis by starting an ADC conversion
        adc_select_input(1); // Select ADC1 (y)
        uint16_t y = adc_read();
        
        // Load the joystick data into the buffer (xLSB, xMSB, yLSB, yMSB)
        out_buf[0] = x;
        out_buf[1] = (x >> 8);
        out_buf[2] = y;
        out_buf[3] = (y >> 8);

        // DEBUG - Set ALL buffer data to the 4-byte joystick data
        for(uint16_t i = 0; i < BUF_LEN; i += 4)
        {
            out_buf[i]   = x;
            out_buf[i+1] = (x >> 8);
            out_buf[i+2] = y;
            out_buf[i+3] = (y >> 8);
        }

        printf("(DEBUG) Outbut buffer set to button state\n");

        printf("X : %i\n", x);
        printf("Y : %i\n", y);

        if(spi.SlaveWrite(out_buf, in_buf, BUF_LEN)) {
            printf("Slave Write Executed\n");
        }
        else {
            printf("Slave Write FAILED\n");
        }

    }

}