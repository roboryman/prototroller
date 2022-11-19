#include "pico/stdlib.h"
#include <stdio.h>
#include "../libraries/SPIMaster.h"

#define RESCAN_BUTTON_PIN 28

/* Globals */
volatile bool rescan = false;
volatile unsigned long time = to_ms_since_boot(get_absolute_time());
const int delay = 50; // Rescan button debounce delay

typedef enum {
    NOT_CONNECTED,
    BUTTON_MODULE,
    JOYSTICK_MODULE
} moduleID_t;

void rescan_callback(uint gpio, uint32_t events)
{
    if((to_ms_since_boot(get_absolute_time()) - time) > delay && !rescan)
    {
        
        time = to_ms_since_boot(get_absolute_time());

        printf("Triggering a rescan...\n");

        rescan = true;
    }
}

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

void rescan_modules(SPIMaster &master)
{
    printf("Identifying Modules...\n");
    printf("======================\n");

    for(uint8_t module = 0; module < MAX_MODULES; module++)
    {
        // Select the current (potentially connected) module
        master.SlaveSelect(module);

        // Attempt to identify module currently selected
        uint8_t identification = master.MasterIdentify();

        printf("Module %u is identified as %u\n", module, identification);
    }

    rescan = false;
}

void init_gpio()
{
    // Initialize GPIO pins for CSNs
    for(uint8_t pin = CSN_START_PIN; pin <= CSN_END_PIN; pin++)
    {
        gpio_init(pin);
        gpio_set_dir(pin, true);
        gpio_set_pulls(pin, false, false);
        gpio_put(pin, 1);
    }

    // Initialize GPIO pin for rescan button
    gpio_init(RESCAN_BUTTON_PIN);
    gpio_set_dir(RESCAN_BUTTON_PIN, false);
    gpio_set_pulls(RESCAN_BUTTON_PIN, false, false);

    // Setup the rescan callback
    gpio_set_irq_enabled_with_callback(
        RESCAN_BUTTON_PIN,
        GPIO_IRQ_EDGE_RISE,
        true,
        &rescan_callback
    );
}

int main() {
    stdio_init_all();

    printf("SPI Master\n");

    // Sleep for module initialization
    sleep_ms(10000);

    SPIMaster master(
        spi_default,
        SPI_TX_PIN,
        SPI_RX_PIN,
        SPI_SCK_PIN,
        SPI_CSN_PIN,
        false
    );

    master.MasterInit();

    // Initialize all GPIO
    init_gpio();

    // Declare and initialize buffers
    uint8_t out_buf[BUF_LEN];
    uint8_t in_buf[BUF_LEN];
    
    // Initial rescan
    rescan_modules(master);

    while(true) {
        if(rescan)
        {
            rescan_modules(master);
        }

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

        //sleep_ms(500);

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

        //sleep_ms(500);


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