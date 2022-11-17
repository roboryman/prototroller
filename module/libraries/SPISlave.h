#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

//-- Pin Definitions --//
#define SPI_TX_PIN  19
#define SPI_SCK_PIN 18
#define SPI_CSN_PIN 17
#define SPI_RX_PIN  16
#define DEFAULT_SPI 0

#define BUF_LEN 0x100

#define PLEASE_IDENTIFY    0xAA

class SPISlave {
    uint8_t inBuf[BUF_LEN];
    uint8_t outBuf[BUF_LEN];
    size_t buffSize;
    uint SPIInst = 0;

    public:
        SPISlave(size_t bufLength);
        void InitComponent(uint SPIInst, uint TX, uint RX, uint SCK, uint CSN, uint bRate);
        bool SlaveWrite(uint8_t data[], size_t dataSize);
        void SlaveWriteIdentifier(uint8_t identifier);
};