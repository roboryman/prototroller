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

    printf("SPI Master\n");

    SPIMaster master(0x4);
    master.InitComponent(DEFAULT_SPI, SPI_TX_PIN, SPI_RX_PIN, SPI_SCK_PIN, SPI_CSN_PIN, 1000*1000);
    
    gpio_init(14);
    gpio_set_dir(14, true);
    gpio_set_pulls(14, false, false);
    gpio_put(14, 1);

    gpio_init(15);
    gpio_set_dir(15, true);
    gpio_set_pulls(15, false, false);
    gpio_put(15, 1);
    
    const float conversion_factor = 3.3f / (1 << 12);
    while(true) {
        gpio_put(14, 0);
        master.masterRead();
        gpio_put(14, 1);
        printbuf(master.inBuf, 1);

        //sleep_ms(500);

        
        gpio_put(15, 0);
        master.masterRead();
        gpio_put(15, 1);

        uint16_t x = (master.inBuf[1] << 8) | master.inBuf[0];
        uint16_t y = (master.inBuf[3] << 8) | master.inBuf[2];

        printf("X Raw value: 0x%04x, voltage: %f V\n", x, x * conversion_factor);
        printf("Y Raw value: 0x%04x, voltage: %f V\n\n", y, y * conversion_factor);
        
        sleep_ms(500);
    }
}