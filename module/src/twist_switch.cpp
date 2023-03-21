#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
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

    // Emable SPI and connect to GPIOs
    spi.SlaveInit();

    // Initialize the ADC/GPIO
    adc_init();
    adc_gpio_init(MODULE_TWIST_SWITCH_ADCPIN);

    // Wait for identification
    spi.SlaveWrite(out_buf, in_buf, BUF_LEN);

    // After identifier is sent, continually send the GPIO state
    while(true)
    {
        // Read the voltage on the wiper by starting an ADC conversion
        adc_select_input(0); // Select ADC0 (x)
        uint16_t wiper = adc_read();
        
        // Load the wiper data into the buffer (LSB, MSB)
        out_buf[0] = wiper;
        out_buf[1] = (wiper >> 8);

        spi.SlaveWrite(out_buf, in_buf, BUF_LEN);
    }

}