#include "SPISlave.h"

SPISlave::SPISlave(spi_inst_t *spi, uint TXPin, uint RXPin, uint SCKPin, uint CSNPin)
{
    this->spi = spi;
    this->TXPin = TXPin;
    this->RXPin = RXPin;
    this->SCKPin = SCKPin;
    this->CSNPin = CSNPin;
}

/* InitComponent
 * Args: SPI Instance (uint), TX Pin (uint), RX Pin (uint), SCK Pin (uint), CSN Pin (uint), Baud Rate (uint)
 * Description: Initialize the SPI, set as slave, and set up GPIO.
 */
void SPISlave::SlaveInit()
{
    
    // Initialize SPI, set as slave, and set format
    /*
    Note:
    We are using CHPA=1 here so we can keep the CSN low during the entire transfer.
    */
    spi_init(spi, BAUD_RATE);
    spi_set_slave(spi, true);
    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);

    // Initalize GPIO
    gpio_set_function(TXPin, GPIO_FUNC_SPI);
    gpio_set_function(RXPin, GPIO_FUNC_SPI);
    gpio_set_function(SCKPin, GPIO_FUNC_SPI);
    gpio_set_function(CSNPin, GPIO_FUNC_SPI);
}

/* SlaveWrite
 * Args: data buffer (uint8_t *), data buffer size (size_t)
 * Description: Write buffer data over the appropriate SPI module.
 *              If the master is requesting identification, return true.
 *              Otherwise, return false.
 */
bool SPISlave::SlaveWrite(uint8_t *out_buf, uint8_t *in_buf, size_t len)
{
    // Ensure the buffer data size is <= buffer size
    // if(buffSize < dataSize) {
    //     return false;
    // }
    
    // Overide the output enable to disable, in case the Pico is not selected but driving the TX pin
    //gpio_set_oeover(TXPin, GPIO_OVERRIDE_LOW);
    //gpio_set_function(TXPin, GPIO_FUNC_NULL); // Uncomment if TX pin still drives low when not selected

    // Check if this Pico slave is selected
    //if(!gpio_get(CSNPin))
    //{
        // Set TX pin override status back to normal
        //gpio_set_oeover(TXPin, GPIO_OVERRIDE_NORMAL);
        //gpio_set_function(TXPin, GPIO_FUNC_SPI); // Uncomment if TX pin still drives low when not selected

        // Read/Write data
        spi_write_read_blocking(spi, out_buf, in_buf, BUF_LEN);
    //}

    return false;
    //return in_buf[0] == PLEASE_IDENTIFY;
}

/* SlaveWriteIdentifier
 * Args: module identifier (uint8_t)
 * Description: Write the module identifier over the appropriate SPI module.
 */
// void SPISlave::SlaveWriteIdentifier(uint8_t identifier) {
//     outBuf[0] = identifier;

//     if(SPIInst) {
//         spi_write_read_blocking(spi1, outBuf, inBuf, BUF_LEN);
//     } else {
//         spi_write_read_blocking(spi0, outBuf, inBuf, BUF_LEN);
//     }
// }

