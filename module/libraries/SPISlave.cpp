#include "SPISlave.h"

SPISlave::SPISlave(spi_inst_t *spi, uint TXPin, uint RXPin, uint SCKPin, uint CSNPin,uint8_t ID)
{
    this->spi = spi;
    this->TXPin = TXPin;
    this->RXPin = RXPin;
    this->SCKPin = SCKPin;
    this->CSNPin = CSNPin;
    this->ID = ID;
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

    gpio_set_oeover(TXPin, GPIO_OVERRIDE_LOW);
}

/* SlaveWrite
 * Args: data buffer (uint8_t *), data buffer size (size_t)
 * Description: Write buffer data over the appropriate SPI module.
 *              If the master requested data or identification, return true.
 *              Otherwise, return false.
 */
bool SPISlave::SlaveWrite(uint8_t *out_buf, uint8_t *in_buf, size_t len)
{
    // Set the last output buffer byte to the verify byte
    out_buf[len-1] = (uint8_t) VERIFY_BYTE;

    uint8_t buf[1];

    // Block until the master sends ready to write signal
    spi_read_blocking(spi, 0x09, buf, 1);
    
    // Data Read Handshake
    if(buf[0] == DATA_REQUEST)
    {
        // Take control of TX line
        gpio_set_oeover(TXPin, GPIO_OVERRIDE_NORMAL);

        //Read/Write data
        spi_write_read_blocking(spi, out_buf, in_buf, BUF_LEN);

        // Overide the output enable to disable, in case the Pico is not selected but driving the TX pin
        gpio_set_oeover(TXPin, GPIO_OVERRIDE_LOW);

        return true;
    }
    // Identification Handshake
    if(buf[0] == PLEASE_IDENTIFY)
    {
        // Take control of TX line
        gpio_set_oeover(TXPin, GPIO_OVERRIDE_NORMAL);
        buf[0] = 0;
        buf[0] = ID;
        
        //Write Identifier 
        spi_write_blocking(spi, buf, 1);

        // Overide the output enable to disable, in case the Pico is not selected but driving the TX pin
        gpio_set_oeover(TXPin, GPIO_OVERRIDE_LOW);

        return true;
    }

    return false;
}