#include "SPIMaster.h"
#include "../../prototroller.h"

SPIMaster::SPIMaster(spi_inst_t *spi, uint TXPin, uint RXPin, uint SCKPin, uint CSNPin, bool externalDecoder)
{
    this->spi = spi;
    this->TXPin = TXPin;
    this->RXPin = RXPin;
    this->SCKPin = SCKPin;
    this->CSNPin = CSNPin;
    this->externalDecoder = externalDecoder;
}

/* MasterInit
 * Initialize the SPI instance using provided values
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

/* MasterReadWrite
 * Send Data Request Handshake, Read Entire Data Buffer after brief delay
 */
bool SPIMaster::MasterReadWrite(uint8_t *out_buf, uint8_t *in_buf, size_t len)
{
    uint8_t buf[1] = {DATA_REQUEST};

    // Write Data Request Handshake
    spi_write_blocking(spi, buf, 1);

    // Wait for Slave to assume control of the TX line
    sleep_us(WAIT_FOR_SLAVE_US);

    // Read Data
    spi_write_read_blocking(spi, out_buf, in_buf, len);

    // Return if valid data is received
    // If module is disconnected, the line would always read 0x00, ...,
    // So best to use a verify value such as 0xAA.
    return in_buf[len-1] == (uint8_t) VERIFY_BYTE;
}

/* MasterIdentify
 * Send Identification Handshake, Return Response Byte
 */
uint8_t SPIMaster::MasterIdentify()
{
    uint8_t buf[1] = {PLEASE_IDENTIFY};

    // Write Identification Request Handshake
    spi_write_blocking(spi, buf, 1);

    // Wait for Slave to assume control of the TX line
    sleep_us(WAIT_FOR_SLAVE_US);

    // Read Identification response
    spi_read_blocking(spi, 0x07, buf, 1);
    
    // Return identification response
    return(buf[0]);
}

/* MasterRead
 * Take CSN and place onto proper GPIO pins
 */
void SPIMaster::SlaveSelect(uint8_t CSN)
{
    if(!externalDecoder)
    {
        // No external active-low decoder for chip selects, so
        // do it internally here (cost: using more GPIO)

        // Ex: CSN 3 (0-24)
        // Pin 0: 1
        // Pin 1: 1
        // Pin 2: 1
        // Pin 3: 0
        // Pin ...: 1

        for(uint8_t pin = MASTER_CSN_START_PIN; pin <= MASTER_CSN_END_PIN; pin++)
        {
            gpio_put(pin, (pin - MASTER_CSN_START_PIN) != CSN);
        }
    }
    else
    {
        // Place each bit of CSN on the appropriate pins to send to decoder
        // This assumes the start pin is the little-end

        // Ex: CSN 3 (0-24)
        // Pin 0: 1
        // Pin 1: 1
        // Pin 2: 0
        // Pin ...: 0

        if(CSN >= 16)
        {
            gpio_put(MASTER_EONA_PIN, 1);
            gpio_put(MASTER_EONB_PIN, 0);
        }
        else if(CSN <= 15)
        {
            gpio_put(MASTER_EONA_PIN, 0);
            gpio_put(MASTER_EONB_PIN, 1);
        }
        else
        {
            gpio_put(MASTER_EONA_PIN, 1);
            gpio_put(MASTER_EONB_PIN, 1);
        }

        for(uint8_t pin = MASTER_A0_PIN; pin >= MASTER_A3_PIN; pin--)
        {
            gpio_put(pin, 0x01 & (CSN >> (MASTER_A0_PIN - pin)));
        }
    }

    // Wait for slave to enter callback
    sleep_us(WAIT_FOR_SLAVE_US);
}