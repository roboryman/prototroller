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
    SPISlave spi(0x100);

    // Initialize logging if enabled
    if(LOGGING)
    {
        stdio_init_all();
    }

    printf("Joystick Module");

    // Emable SPI0 at 1 MHz and connect to GPIOs
    spi.InitComponent
    (
        DEFAULT_SPI,
        SPI_TX_PIN,
        SPI_RX_PIN,
        SPI_SCK_PIN,
        SPI_CSN_PIN,
        1000 * 1000
    );

    // Initialize the ADC/GPIO
    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);

    // First data sent over SPI is the module identifier
    spi.SlaveWriteIdentifier(JOYSTICK_MODULE_ID);

    // Declare a buffer to use in our infinite loop
    uint8_t buf[4];

    // Declare x-axis and y-axis data holders
    uint16_t x;
    uint16_t y;

    // After identifier is sent, continually send the GPIO state
    while(true)
    {
        // Read the X-axis by starting an ADC conversion
        adc_select_input(0); // Select ADC0 (x)
        x = adc_read();

        // Read the Y-axis by starting an ADC conversion
        adc_select_input(1); // Select ADC1 (y)
        y = adc_read();
        
        // Load the data into a 4-byte data buffer (xLSB, xMSB, yLSB, yMSB)
        buf[0] = x;
        buf[1] = (x >> 8);
        buf[2] = y;
        buf[3] = (y >> 8);

        // Log the joystick data (no floating point math here - expensive, and we care about latency)
        printf("X : %i\n", x);
        printf("Y : %i\n", y);

        // Write the joystick data, and re-send module identifier if requested
        while(spi.SlaveWrite(buf, 4))
        {
            spi.SlaveWriteIdentifier(JOYSTICK_MODULE_ID);
        }
    }

}