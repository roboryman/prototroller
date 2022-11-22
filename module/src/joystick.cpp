#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
#include "../libraries/Component.h"
#include "../libraries/SPISlave.h"

#define VRX_PIN 26
#define VRY_PIN 27

Component joystick;
SPISlave spi(
    spi_default,
    SPI_TX_PIN,
    SPI_RX_PIN,
    SPI_SCK_PIN,
    SPI_CSN_PIN,
    JOYSTICK_IDENTITY
);

// Declare and initialize buffers
uint8_t out_buf[BUF_LEN] = {0};
uint8_t in_buf[BUF_LEN] = {0};

// uint8_t out_buf_1[BUF_LEN] = {0x77};
// uint8_t in_buf_1[BUF_LEN] = {0x77};

// volatile bool buf_flag = false;

// TODO: Figure out why this callback is screwing up!
// void selected_callback(uint gpio, uint32_t events)
// {
//     printf("Entered callback\n");
//     if(buf_flag)
//     {
//         if(spi.SlaveWrite(out_buf, in_buf, BUF_LEN)) {
//             //printf("Slave Write Executed\n");
//         }
//         else {
//             //printf("Slave Write FAILED\n");
//         }
//     }
//     else
//     {
//         if(spi.SlaveWrite(out_buf_1, in_buf_1, BUF_LEN)) {
//             //printf("Slave Write Executed (1)\n");
//         }
//         else {
//             //printf("Slave Write FAILED (1)\n");
//         }
//     }
//     printf("Leaving callback\n");

//     //gpio_set_irq_enabled(SPI_CSN_PIN, GPIO_IRQ_EDGE_FALL, false);
//     irq_clear(IO_IRQ_BANK0);

//     while(!gpio_get(SPI_CSN_PIN));
// }

int main() {
    //board_init();

    //stdio_init_all();

    printf("JOYSTICK MODULE\n");

    // Setup the chip select callback
    // gpio_set_irq_enabled_with_callback(
    //     SPI_CSN_PIN,
    //     GPIO_IRQ_EDGE_FALL,
    //     false,
    //     &selected_callback
    // );

    // Emable SPI0 at 1 MHz and connect to GPIOs
    spi.SlaveInit();

    printf("Slave initialized.\n");

    // Initialize the ADC/GPIO
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    printf("ADC initialized.\n");

    printf("Wait for ID.\n");
    spi.SlaveWrite(out_buf, in_buf, BUF_LEN);

    // After identifier is sent, continually send the GPIO state
    while(true)
    {
        // Read the X-axis by starting an ADC conversion
        adc_select_input(0); // Select ADC0 (x)
        uint16_t x = adc_read();

        // Read the Y-axis by starting an ADC conversion
        adc_select_input(1); // Select ADC1 (y)
        uint16_t y = adc_read();
        
        // buf_flag = false;

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

        // buf_flag = true;

        //printf("X : %i\n", x);
        //printf("Y : %i\n", y);

        spi.SlaveWrite(out_buf, in_buf, BUF_LEN);

        // if(spi.SlaveWrite(out_buf, in_buf, BUF_LEN)) {
        //     printf("Slave Write Executed\n");
        // }
        // else {
        //     printf("Slave Write FAILED\n");
        // }
    }

}