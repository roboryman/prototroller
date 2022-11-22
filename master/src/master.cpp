#include "pico/stdlib.h"
#include "pico/stdio.h"
//#include <stdio.h>
#include <stdlib.h>
#include "../libraries/SPIMaster.h"
#include "usb_descriptors.h"
#include "tusb.h"
#include "bsp/board.h"

#define MOUSE_SENS 5
#define RESCAN_BUTTON_PIN 28

/* Enumerations */
enum
{
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

typedef enum {
    NOT_CONNECTED,
    BUTTON_MODULE,
    JOYSTICK_MODULE
} moduleID_t;

const char module_names[][20] =
    {
        "NOT CONNECTED",
        "BUTTON MODULE",
        "JOYSTICK MODULE"
    };

/* Rescan debouncing */
volatile bool rescan = false;
volatile unsigned long time = to_ms_since_boot(get_absolute_time());
const int delay = 50; // Rescan button debounce delay

/* TinyUSB mounted blink interval */
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

/* Data holders for main app => TinyUSB */
int8_t delta_x;
int8_t delta_y;
uint8_t buttons;

/* Prototypes */
void led_blinking_task(void);
void hid_task(void);

/* SPI Master */
SPIMaster master(
        spi_default,
        SPI_TX_PIN,
        SPI_RX_PIN,
        SPI_SCK_PIN,
        SPI_CSN_PIN,
        false
    );

/* Buffers */
uint8_t out_buf[BUF_LEN];
uint8_t in_buf[BUF_LEN];

/* Module state data store */
moduleID_t module_IDs[MAX_MODULES] = {NOT_CONNECTED};


/* rescan_callback
 * Args: gpio, events
 * Description: Interrupt for the rescan button.
 * Sets the rescan flag so a rescan is executed on the next
 * modules_task() call.
 * Debounced to the value of delay.
 */
void rescan_callback(uint gpio, uint32_t events)
{
    if((to_ms_since_boot(get_absolute_time()) - time) > delay && !rescan)
    {
        // Set the interrupt time
        time = to_ms_since_boot(get_absolute_time());

        // Set the rescan flag
        rescan = true;
    }
}

/* printbuf
 * Args: buffer to print, length of buffer
 * Description: Prints a buffer.
 * WARNING: THIS WILL IMPACT SPI OPERATION ON THE SLAVE.
 * PROBABLY THE MASTER TOO.
 */
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

/* rescan_modules
 * Args: None
 * Description: Rescan the protogrid for active modules.
 * Update the module identification to not connected
 * or a unique module specifier.
 */
void rescan_modules()
{
    printf("[!] Rescanning for active modules...\n");

    for(uint8_t module = 0; module < MAX_MODULES; module++)
    {
        // Select the current (potentially connected) module
        master.SlaveSelect(module);

        // Attempt to identify module currently selected
        moduleID_t identification = (moduleID_t) master.MasterIdentify();

        // Deselect the module
        master.SlaveSelect(NO_SLAVE_SELECTED_CSN);

        // Update the module identification
        module_IDs[module] = identification;

        char status = (identification == NOT_CONNECTED) ? 'x' : '!';
        printf("[%c] Module %u is identified as ", status, module);
        printf(module_names[identification]);
        printf(".\n");
    }

    // Rescan finished, so reset the flag
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

    //Setup the rescan callback
    gpio_set_irq_enabled_with_callback(
        RESCAN_BUTTON_PIN,
        GPIO_IRQ_EDGE_RISE,
        true,
        &rescan_callback
    );
}

void modules_task()
{
    if(rescan)
    {
        rescan_modules();
    }
    
    uint8_t buttonIndex = 0;
    buttons = 0;
    for(uint8_t module = 0; module < MAX_MODULES; module++)
    {
        if(module_IDs[module])
        {
            // Current module is connected.

            // Process module input into out_buf
            // ... TBD

            // Select the module
            master.SlaveSelect(module);

            // Read module data
            master.MasterRead(out_buf, in_buf, BUF_LEN);

            // Deselect the module
            master.SlaveSelect(NO_SLAVE_SELECTED_CSN);

            // Process module output from in_buf
            switch(module_IDs[module])
            {
                case BUTTON_MODULE:
                    {
                        //printbuf(in_buf, BUF_LEN);
                        if(in_buf[0] == 0x00) {
                            buttons = (1 << buttonIndex); 
                        }
                        buttonIndex += 1;
                        printf("Button: %d\n", in_buf[0]);
                        printf("Button Mask: %d\n", buttons);
                    }

                    break;
                
                case JOYSTICK_MODULE:
                    {
                        uint16_t x = (in_buf[1] << 8) | in_buf[0];
                        uint16_t y = (in_buf[3] << 8) | in_buf[2];

                        uint16_t offset = 200;
                        delta_x = ((x + offset) >> 9)-4;
                        delta_y = ((y + offset) >> 9)-4;

                        //printbuf(in_buf, BUF_LEN);

                        printf("Delta X: %d\n", delta_x);
                        printf("Delta Y: %d\n\n", delta_y);
                    }
                    break;

                default:
                    // Remove from data store ...?
                    printf("Unknown module: ");
                    printf(module_names[module_IDs[module]]);
                    printf("\n");

                    break;
            }

            // If spi is acting up... uncomment
            //sleep_ms(10);
        }
    }
}

int main() {
    // Sleep for module initialization and time to setup console
    sleep_ms(5000);
    
    //board_init();
    tusb_init();

    //stdio_init_all(); // Use w/o TinyUSB
    stdio_usb_init(); // Use w/ TinyUSB

    // NOTE: sleeps after tusb_init() appear to cause the USB mount to fail.
    //sleep_ms(5000);

    printf("MASTER INITIALIZATION PROCEDURE\n");
    printf("===============================\n");

    // Initialize SPI Stuff
    master.MasterInit();

    printf("[!] Master SPI Initialized\n");

    // Initialize all GPIO
    init_gpio();

    printf("[!] Master GPIO Initialized\n");

    // Initial rescan
    rescan_modules();

    //sleep_ms(5000);
    
    while(true) {
        modules_task();
        hid_task(); // <== SPI stuff to go in here? Hmm....
        //led_blinking_task(); // Blink the LED based on mounted status
        tud_task(); // tinyusb device task
    }
}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// static void send_hid_report(uint8_t report_id, uint32_t btn)
// {
//   // skip if hid is not ready yet
//   if (!tud_hid_ready()) {
//     return;  
//   }
//   // send mouse report data
//   tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta_x, delta_y, 0, 0);
// }

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return; // not enough time

    start_ms += interval_ms;

    // Remote wakeup
    if (tud_suspended())
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }
    // else
    // {
    //     // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    //     send_hid_report(REPORT_ID_MOUSE, 0);


    // }

    // skip if hid is not ready yet
    if (tud_hid_ready()) {
        // Format and send HID report data

        // Mouse (from joystick and buttons)
        tud_hid_mouse_report(REPORT_ID_MOUSE, buttons, delta_x, delta_y, 0, 0);
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
// void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
// {
//   (void)instance;
//   (void)len;

//   uint8_t next_report_id = report[0] + 1;

//   if (next_report_id < REPORT_ID_COUNT)
//   {
//     send_hid_report(next_report_id, 0);
//   }
// }

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD)
        {
            // bufsize should be (at least) 1
            if ( bufsize < 1 ) return;

            uint8_t const kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
            {
                // Capslock On: disable blink, turn led on
                blink_interval_ms = 0;
                board_led_write(true);
            }
            else
            {
                // Caplocks Off: back to normal blink
                board_led_write(false);
                blink_interval_ms = BLINK_MOUNTED;
            }
        }
    }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // blink is disabled
    if (!blink_interval_ms) return;

    // Blink every interval ms
    if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}