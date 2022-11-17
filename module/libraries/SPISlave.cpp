#include "SPISlave.h"

SPISlave::SPISlave(size_t bufLength){
    buffSize = bufLength;
}

/* InitComponent
 * Args: SPI Instance (uint), TX Pin (uint), RX Pin (uint), SCK Pin (uint), CSN Pin (uint), Baud Rate (uint)
 * Description: Initialize the SPI, set as slave, and set up GPIO.
 */
void SPISlave::InitComponent(uint SPIInst, uint TX, uint RX, uint SCK, uint CSN, uint bRate) {
    
    // Initialize SPI and set as slave
    if(SPIInst) {
        spi_init(spi1, bRate);
        spi_set_slave(spi1, true);
    } else {
        spi_init(spi0, bRate);
        spi_set_slave(spi0, true);    
    }

    // Initalize GPIO
    gpio_set_function(TX, GPIO_FUNC_SPI);
    gpio_set_function(RX, GPIO_FUNC_SPI);
    gpio_set_function(SCK, GPIO_FUNC_SPI);
    gpio_set_function(CSN, GPIO_FUNC_SPI);
}

/* SlaveWrite
 * Args: data buffer (uint8_t *), data buffer size (size_t)
 * Description: Write buffer data over the appropriate SPI module.
 *              If the master is requesting identification, return true.
 *              Otherwise, return false.
 */
bool SPISlave::SlaveWrite(uint8_t data[], size_t dataSize) {
    // Ensure the buffer data size is <= buffer size
    if(buffSize < dataSize) {
        return false;
    }

    for (size_t i = 0; i < dataSize; i++) {
        outBuf[i] = data[i];
    }

    if(SPIInst) {
        spi_write_read_blocking(spi1, outBuf, inBuf, BUF_LEN);
    } else {
        spi_write_read_blocking(spi0, outBuf, inBuf, BUF_LEN);
    }

    return inBuf[0] == PLEASE_IDENTIFY;
}

/* SlaveWriteIdentifier
 * Args: module identifier (uint8_t)
 * Description: Write the module identifier over the appropriate SPI module.
 */
void SPISlave::SlaveWriteIdentifier(uint8_t identifier) {
    outBuf[0] = identifier;

    if(SPIInst) {
        spi_write_read_blocking(spi1, outBuf, inBuf, BUF_LEN);
    } else {
        spi_write_read_blocking(spi0, outBuf, inBuf, BUF_LEN);
    }
}

