#include "pico/stdlib.h"
#include <stdio.h>
#include "../libraries/SPIComponent.h"



int main() {
    stdio_init_all();

    SPIComponent slave(0x100);
    slave.InitComponent(DEFAULT_SPI,SPI_MISO_PIN,SPI_MOSI_PIN,SPI_SCK_PIN,SPI_SS_PIN,1000*1000);
    uint8_t data[] = {1,2,3,4,5};
    while(1){
        slave.slaveWrite(data,5);
        sleep_ms(10 * 1000);
    }
}