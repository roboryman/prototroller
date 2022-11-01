#include "SPIComponent.h"

SPIComponent::SPIComponent(size_t bufLength){
    buffSize = bufLength;
}
void SPIComponent::InitComponent(uint SPIInst, uint MISO, uint MOSI, uint SCK, uint SS, uint bRate){
    
    if(SPIInst){
        spi_init(spi1, bRate);
        spi_set_slave(spi1, true);
    } else {
        spi_init(spi1, bRate);
        spi_set_slave(spi1, true);    
    }

        //Init GPIO
        gpio_set_function(MISO, GPIO_FUNC_SPI);
        gpio_set_function(MOSI, GPIO_FUNC_SPI);
        gpio_set_function(SCK, GPIO_FUNC_SPI);
        gpio_set_function(SS, GPIO_FUNC_SPI);
}

void SPIComponent::slaveWrite(uint8_t data[], size_t dataSize){
    if(buffSize < dataSize){
        return;
    }
    for (size_t i = 0; i < dataSize; i++){
        outBuf[i] = data[i];
    }
    if(SPIInst){
        spi_write_read_blocking(spi1, outBuf, inBuf, BUF_LEN);
    } else {
    spi_write_read_blocking(spi0, outBuf, inBuf, BUF_LEN);
    }
}




