#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

//-- Handshake Identifiers --//
#define PLEASE_IDENTIFY 0xAA
#define DATA_REQUEST    0xBB

//-- Module Identifiers --//
#define VERIFY_BYTE         0x55
#define BUTTON_IDENTITY     0x01
#define JOYSTICK_IDENTITY   0x02

class SPISlave
{
    // uint8_t inBuf[BUF_LEN];
    // uint8_t outBuf[BUF_LEN];
    // size_t buffSize;
    spi_inst_t *spi;
    uint TXPin;
    uint RXPin;
    uint SCKPin;
    uint CSNPin;
    uint8_t ID;

    public:
        SPISlave(spi_inst_t *spi, uint TXPin, uint RXPin, uint SCKPin, uint CSNPin,uint8_t ID);
        void SlaveInit();
        bool SlaveWrite(uint8_t *out_buf, uint8_t *in_buf, size_t len);
        //void SlaveWriteIdentifier(uint8_t identifier);
};