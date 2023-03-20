#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

//-- Slave Selection --//
#define MAX_MODULES             20
#define NO_SLAVE_SELECTED_CSN   34

//-- Handshake Identifiers --//
#define VERIFY_BYTE         0x55
#define PLEASE_IDENTIFY     0xAA
#define DATA_REQUEST        0xBB
#define WAIT_FOR_SLAVE_US   100

class SPIMaster
{
    // size_t buffSize;
    // uint SPIInst = 0;
    // uint8_t inBuf[BUF_LEN];
    // uint8_t outBuf[BUF_LEN];

    spi_inst_t *spi;
    uint TXPin;
    uint RXPin;
    uint SCKPin;
    uint CSNPin;
    bool externalDecoder;

    public:
        SPIMaster(spi_inst_t *spi, uint TXPin, uint RXPin, uint SCKPin, uint CSNPin, bool externalDecoder);
        void MasterInit();
        bool MasterRead(uint8_t *out_buf, uint8_t *in_buf, size_t len);
        uint8_t MasterIdentify();
        void SlaveSelect(uint8_t CSN);
};