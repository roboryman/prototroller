#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

//-- Pin Definitions --//
#define SPI_TX_PIN   19
#define SPI_SCK_PIN  18
#define SPI_CSN_PIN  17
#define SPI_RX_PIN   16
#define CSN_END_PIN   15
#define CSN_START_PIN 13

//-- SPI Format --//
#define BAUD_RATE 1000*1000
#define BUF_LEN 0x100

//-- Slave Selection --//
#define MAX_MODULES 25
#define NO_SLAVE_SELECTED_CSN 25

//-- Handshake Identifiers --//
#define PLEASE_IDENTIFY 0xAA
#define DATA_REQUEST 0xBB
#define WAIT_FOR_SLAVE_US 100

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
        void MasterRead(uint8_t *out_buf, uint8_t *in_buf, size_t len);
        uint8_t MasterIdentify();
        void SlaveSelect(uint8_t CSN);
};