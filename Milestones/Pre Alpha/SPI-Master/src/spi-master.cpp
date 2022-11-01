#include "pico/stdlib.h"
#include <stdio.h>
#include "../libraries/SPIMaster.h"


void printbuf(uint8_t buf[], size_t len) {
    int i;
    for (i = 0; i < len; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }

    // append trailing newline if there isn't one
    if (i % 16) {
        putchar('\n');
    }
}

int main() {
    stdio_init_all();

    SPIMaster master(0x100);
    master.InitComponent(DEFAULT_SPI,SPI_MISO_PIN,SPI_MOSI_PIN,SPI_SCK_PIN,SPI_SS_PIN,1000*1000);
    uint8_t data[] = {1,2,3,4,5};
    while(1){
        master.masterRead();
        printbuf(master.inBuf,BUF_LEN);
        sleep_ms(10 * 1000);
    }
}