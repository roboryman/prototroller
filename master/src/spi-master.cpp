#include "pico/stdlib.h"
#include <stdio.h>
#include "../libraries/SPIMaster.h"


void printbuf(uint8_t buf[], size_t len)
{
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

    SPIMaster master(
        spi_default,
        SPI_TX_PIN,
        SPI_RX_PIN,
        SPI_SCK_PIN,
        SPI_CSN_PIN
    );

    master.MasterInit();

    // Initialize GPIO pins for CSN decoder
    gpio_init(13);
    gpio_set_dir(13, true);
    gpio_set_pulls(13, false, false);
    gpio_put(13, 0);
    
    gpio_init(14);
    gpio_set_dir(14, true);
    gpio_set_pulls(14, false, false);
    gpio_put(14, 0);

    gpio_init(15);
    gpio_set_dir(15, true);
    gpio_set_pulls(15, false, false);
    gpio_put(15, 0);

    // Declare and initialize buffers
    uint8_t out_buf[BUF_LEN];
    uint8_t in_buf[BUF_LEN];
    
    //const float conversion_factor = 3.3f / (1 << 12);
    while(true) {
        // Select the button module, read module data, and print
        //master.SlaveSelect(0);
        gpio_put(13, 0);
        gpio_put(14, 1);
        //sleep_ms(500); // DEBUG
        master.MasterRead(out_buf, in_buf, BUF_LEN);
        //master.SlaveSelect(NO_SLAVE_SELECTED);
        gpio_put(13, 1);
        gpio_put(14, 1);
        printf("BUTTON PACKET\n");
        printbuf(in_buf, BUF_LEN);
        printf("\n");

        sleep_ms(2000);

        // Select the joystick module, read module data, and print
        //master.SlaveSelect(1);
        gpio_put(13, 1);
        gpio_put(14, 0);
        //sleep_ms(500); // DEBUG
        master.MasterRead(out_buf, in_buf, BUF_LEN);
        //master.SlaveSelect(NO_SLAVE_SELECTED);
        gpio_put(13, 1);
        gpio_put(14, 1);
        printf("JOYSTICK PACKET\n");
        printbuf(in_buf, BUF_LEN);
        printf("\n");

        sleep_ms(2000);


        // gpio_put(14, 0);
        // master.masterRead();
        // gpio_put(14, 1);
        // printbuf(master.inBuf, 1);

        // //sleep_ms(500);

        
        // gpio_put(15, 0);
        // master.masterRead();
        // gpio_put(15, 1);

        // uint16_t x = (master.inBuf[1] << 8) | master.inBuf[0];
        // uint16_t y = (master.inBuf[3] << 8) | master.inBuf[2];

        // printf("X Raw value: 0x%04x, voltage: %f V\n", x, x * conversion_factor);
        // printf("Y Raw value: 0x%04x, voltage: %f V\n\n", y, y * conversion_factor);
        
        // sleep_ms(500);
    }
}