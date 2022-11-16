#include "SPIMaster.h"

SPIMaster::SPIMaster(size_t bufLength){
    buffSize = bufLength;
}
void SPIMaster::InitComponent(uint SPIInst, uint MISO, uint MOSI, uint SCK, uint SS, uint bRate){
    
    if(SPIInst){
        spi_init(spi1, bRate);
        spi_set_slave(spi1, false);
    } else {
        spi_init(spi0, bRate);
        spi_set_slave(spi0, false);    
    }

        //Init GPIO
        gpio_set_function(MISO, GPIO_FUNC_SPI);
        gpio_set_function(MOSI, GPIO_FUNC_SPI);
        gpio_set_function(SCK, GPIO_FUNC_SPI);
        gpio_set_function(SS, GPIO_FUNC_SPI);
}

void SPIMaster::masterRead(){
    if(SPIInst){
        spi_write_read_blocking(spi1, outBuf, inBuf, BUF_LEN);
    } else {
    spi_write_read_blocking(spi0, outBuf, inBuf, BUF_LEN);
    }
}




