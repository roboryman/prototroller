#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

//-- Pin Definitions --//
//#define SPI_SS_PIN 21
//#define SPI_MISO_PIN 20
//#define SPI_MOSI_PIN 19
//#define SPI_SCK_PIN  18
//#define DEFAULT_SPI 0

//#define BUF_LEN 0x100
class CAL {
    //size_t buffSize;
    //uint SPIInst = 0;
    public:
        //SPIMaster(size_t bufLength);
        //void InitComponent(uint SPIInst, uint MISO, uint MOSI, uint SCK, uint SS, uint bRate);
        //void masterRead();
        //uint8_t inBuf[BUF_LEN];
        //uint8_t outBuf[BUF_LEN];

        CAL();
        void InitGPIO(uint pin, bool pullUp);
        uint ReadGPIO(uint pin);
        void WriteGPIO(uint pin, uint value);

};