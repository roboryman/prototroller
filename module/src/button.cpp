#include <stdio.h>
#include "pico/stdlib.h"
#include "../libraries/CAL.h"
#include "../libraries/SPISlave.h"

#define GPIO_PIN 20

int main() {
    CAL cal;

    SPISlave slave(0x100);
    slave.InitComponent(DEFAULT_SPI, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SCK_PIN, SPI_SS_PIN, 1000*1000);

    // Initialize GPIO pin 20, active internal pull-up resistor
    cal.InitGPIO(GPIO_PIN, true);

    while(1)
    {
        // Read the GPIO value
        uint8_t data = cal.ReadGPIO(GPIO_PIN);

        // Write the data over SPI
        while(1) {
            slave.slaveWrite(&data, 1);
            //sleep_ms(10 * 1000);
        }
    }
}