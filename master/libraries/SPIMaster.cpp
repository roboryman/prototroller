#include "SPIMaster.h"

SPIMaster::SPIMaster(spi_inst_t *spi, uint TXPin, uint RXPin, uint SCKPin, uint CSNPin)
{
    this->spi = spi;
    this->TXPin = TXPin;
    this->RXPin = RXPin;
    this->SCKPin = SCKPin;
    this->CSNPin = CSNPin;
}
/* MasterInit
 * Args: None
 * Description: Initialize the SPI instance using provided values
 */
void SPIMaster::MasterInit()
{
    
    spi_init(spi, BAUD_RATE);
    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);

    // Initalize GPIO
    gpio_set_function(TXPin, GPIO_FUNC_SPI);
    gpio_set_function(RXPin, GPIO_FUNC_SPI);
    gpio_set_function(SCKPin, GPIO_FUNC_SPI);
    gpio_set_function(CSNPin, GPIO_FUNC_SPI); // Comment? May not need decoder w/ with CHPA=1 (for prototype)
}
/* MasterRead
 * Args: 
 * Description: Send Data Request Handshake, Read Entire Data Buffer after brief delay
 */
void SPIMaster::MasterRead(uint8_t *out_buf, uint8_t *in_buf, size_t len)
{
    uint8_t buf[1] = {DATA_REQUEST};
    //Write Data Request Handshake
    spi_write_blocking(spi, buf, 1);
    //Wait for Slave to assume control of the TX line
    sleep_us(100);
    //Read Data
    spi_write_read_blocking(spi, out_buf, in_buf, len);
}

/* MasterRead
 * Args: CSN (uint8_t)
 * Description: Take CSN and place each bit onto proper GPIO pin
 */
void SPIMaster::SlaveSelect(uint8_t CSN)
{
    // Place each bit of CSN on the appropriate pins to send to decoder
    // This assumes the start pin is the little-end
    for(uint8_t i = CSN_START_PIN; i <= CSN_END_PIN; i++)
    {
        gpio_put(i, 0xFE & (CSN >> (i - CSN_START_PIN)));
    }
}

/* MasterIdentify
 * Args: None
 * Description: Send Identification Handshake, Return Response Byte
 */
uint8_t SPIMaster::MasterIdentify()
{
   uint8_t buf[1] = {PLEASE_IDENTIFY};
    spi_write_blocking(spi, buf, 1);
    //Wait for Slave to assume control of the TX line
    sleep_us(100);
    //Read Data
    spi_read_blocking(spi,0x07,buf, 1);
    
    return(buf[0]);

}