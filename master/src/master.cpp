#include "pico/stdlib.h"
#include "pico/stdio.h"
//#include <stdio.h>
#include <stdlib.h>
#include "../libraries/SPIMaster.h"
#include "usb_descriptors.h"
#include "tusb.h"
#include "bsp/board.h"
#include "../../prototroller.h"

// Uncomment to run in debug mode
#define DEBUG 0

//--------------------------------------------------------------------+
// Types and Enums
//--------------------------------------------------------------------+

// Enum for times
enum
{
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

// Module ID type
typedef enum {
    DISCONNECTED = 0,
    BUTTON_MODULE,
    JOYSTICK_MODULE
} moduleID_t;

// Module string descriptors matching indexing for moduleID_t
const char module_names[][20] =
{
    "DISCONNECTED",
    "BUTTON MODULE",
    "JOYSTICK MODULE"
};

// Prototroller HID Gamepad Report
// Must support the maximum number of each input module per column
typedef struct
{
    uint16_t    digitals;   // Digital inputs (momentary button, maintained button, D-pad, XYAB, etc.)
    int16_t     analogs[8]; // Analog inputs (joystick, slider, twist switch, etc.)
} gamepad_report_t;


//--------------------------------------------------------------------+
// Globals
//--------------------------------------------------------------------+

// Gamepad reports for each column
gamepad_report_t gamepad_report_col_0;
gamepad_report_t gamepad_report_col_1;
gamepad_report_t gamepad_report_col_2;
gamepad_report_t gamepad_report_col_3;
gamepad_report_t gamepad_report_col_4;

// SPI Master
SPIMaster master(
    spi_default,
    MASTER_SPI_TX_PIN,
    MASTER_SPI_RX_PIN,
    MASTER_SPI_SCK_PIN,
    MASTER_SPI_CSN_PIN,
    false
);

// SPI Transaction Buffers
uint8_t out_buf[BUF_LEN];
uint8_t in_buf[BUF_LEN];

// Module Identification Data Store
moduleID_t module_IDs[MAX_MODULES] = {DISCONNECTED};

// Rescan ISR flag
volatile bool rescan = false;

// Rescan debounce timer
volatile unsigned long time = to_ms_since_boot(get_absolute_time());

// Rescan debounce delay (in ms)
const int delay = 50;

// Blink interval in ms for LED blinking task
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

//--------------------------------------------------------------------+
// Prototypes
//--------------------------------------------------------------------+

void printbuf(uint8_t buf[], size_t len);
void rescan_modules();
void init_gpio();
void send_hid_report(uint8_t report_id);
void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);
void tud_resume_cb(void);
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len);
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);
void rescan_cb(uint gpio, uint32_t events);
void hid_task(void);
void led_blinking_task(void);
void modules_task(void);

//--------------------------------------------------------------------+
// Helpers
//--------------------------------------------------------------------+

// Outputs contents of a buffer to standard output.
// WARNING: THIS WILL IMPACT SPI OPERATION.
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

// Rescans for connected modules and update identification.
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

        // Update the module identification in the internal data store
        module_IDs[module] = identification;

        char status = (identification == DISCONNECTED) ? 'x' : '!';

        printf("[%c] Module %u is identified as ", status, module);
        printf(module_names[identification]);
        printf(".\n");
    }

    // Rescan finished, so reset the flag
    rescan = false;
}

// Initializes GPIO and setup interrupts
void init_gpio()
{
    // Initialize GPIO for SPI CSNs
    for(uint8_t pin = MASTER_CSN_START_PIN; pin <= MASTER_CSN_END_PIN; pin++)
    {
        gpio_init(pin);
        gpio_set_dir(pin, true);
        gpio_set_pulls(pin, false, false);
        gpio_put(pin, 1);
    }

    // Initialize GPIO for rescan button
    gpio_init(MASTER_RESCAN_PIN);
    gpio_set_dir(MASTER_RESCAN_PIN, false);
    gpio_set_pulls(MASTER_RESCAN_PIN, false, false);

    // Initialize GPIO for LEDs
    gpio_init(MASTER_LED_R_PIN);
    gpio_set_dir(MASTER_LED_R_PIN, GPIO_OUT);
    gpio_set_pulls(MASTER_LED_R_PIN, false, false);
    gpio_put(MASTER_LED_R_PIN, 1);
    gpio_init(MASTER_LED_G_PIN);
    gpio_set_dir(MASTER_LED_G_PIN, GPIO_OUT);
    gpio_set_pulls(MASTER_LED_G_PIN, false, false);
    gpio_put(MASTER_LED_G_PIN, 1);
    gpio_init(MASTER_LED_B_PIN);
    gpio_set_dir(MASTER_LED_B_PIN, GPIO_OUT);
    gpio_set_pulls(MASTER_LED_B_PIN, false, false);
    gpio_put(MASTER_LED_B_PIN, 1);

    // Setup the rescan callback
    gpio_set_irq_enabled_with_callback(
        MASTER_RESCAN_PIN,
        GPIO_IRQ_EDGE_RISE,
        true,
        &rescan_cb
    );

    #if defined(DEBUG)
    gpio_init(15);
    gpio_set_dir(15, false);
    gpio_set_pulls(15, false, false);
    #endif
}

// Send a gamepad report for the report id / column
void send_hid_report(uint8_t report_id)
{
    // Skip sending this report if the HID endpoint is not ready
    if ( !tud_hid_ready() ) return;

    // Send the appropraite gamepad report based on the report ID (column)
    switch(report_id)
    {
        case REPORT_ID_COLUMN_0:
        {
            tud_hid_report(REPORT_ID_COLUMN_0, &gamepad_report_col_0, sizeof(gamepad_report_col_0));
        }
        break;

        case REPORT_ID_COLUMN_1:
        {
            tud_hid_report(REPORT_ID_COLUMN_1, &gamepad_report_col_1, sizeof(gamepad_report_col_1));
        }
        break;

        case REPORT_ID_COLUMN_2:
        {
            tud_hid_report(REPORT_ID_COLUMN_2, &gamepad_report_col_2, sizeof(gamepad_report_col_2));
        }
        break;

        case REPORT_ID_COLUMN_3:
        {
            tud_hid_report(REPORT_ID_COLUMN_3, &gamepad_report_col_3, sizeof(gamepad_report_col_3));
        }
        break;

        case REPORT_ID_COLUMN_4:
        {
            tud_hid_report(REPORT_ID_COLUMN_4, &gamepad_report_col_4, sizeof(gamepad_report_col_4));
        }
        break;

        default: break;
    }
}


//--------------------------------------------------------------------+
// Callbacks
//--------------------------------------------------------------------+

// Invoked when USB device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when USB device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when USB bus is suspended
// remote_wakeup_en : if host allow us to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void)remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when USB bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when a column report is successfully sent to host
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len)
{
    (void)instance;
    (void)len;

    uint8_t next_report_id = report[0] + 1u;

    if (next_report_id < REPORT_ID_COLUMN_COUNT)
    {
        send_hid_report(next_report_id);
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    // TODO not implemented
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
    // TODO not implemented
    (void) instance;

    /*
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
    */

}

// Invoked when the user presses rescan button. Debounced to the value of delay.
void rescan_cb(uint gpio, uint32_t events)
{
    if((to_ms_since_boot(get_absolute_time()) - time) > delay && !rescan)
    {
        // Set the interrupt time
        time = to_ms_since_boot(get_absolute_time());

        // Set the rescan interrupt flag
        rescan = true;
    }
}


//--------------------------------------------------------------------+
// Tasks
//--------------------------------------------------------------------+

// Task to kick-off the HID report chain
void hid_task(void)
{
    // Poll every 10ms
    // Must be longer than it takes to send all 5 reports
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if ( board_millis() - start_ms < interval_ms) return;

    start_ms += interval_ms;

    // Remote wakeup
    if ( tud_suspended() )
    {
        // Wake up host if we are in suspend mode and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }
    else
    {
        // Kick-off the report chain here, starting with the first column
        send_hid_report(REPORT_ID_COLUMN_0);
    }
}

// Task to blink an LED
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

    printf("LED blinked!\n");
}

// Task to handshake and exchange data with modules over SPI, rescan, update gamepad reports, etc.
void modules_task(void)
{
    // If the user executed a rescan interrupt, do that here
    if(rescan)
    {
        rescan_modules();
    }
    
    // Clear the gamepad report structure
    memset(&gamepad_report_col_0, 0, sizeof(gamepad_report_t));
    memset(&gamepad_report_col_1, 0, sizeof(gamepad_report_t));
    memset(&gamepad_report_col_2, 0, sizeof(gamepad_report_t));
    memset(&gamepad_report_col_3, 0, sizeof(gamepad_report_t));
    memset(&gamepad_report_col_4, 0, sizeof(gamepad_report_t));

    // Initially no connected modules
    bool connectedModules = false;

    // Track how many specific modules as we poll each slot
    uint8_t digitalCount = 0;
    uint8_t analogCount = 0;

    // The gamepad report for the current column
    gamepad_report_t *column_report = &gamepad_report_col_0;

    for(uint8_t module = 0; module < MAX_MODULES; module++)
    {
        // Reset digital and analog count for each column report
        if(module % 4 == 0)
        {
            digitalCount = 0;
            analogCount  = 0;
        }

        // If this is the beginning of a new column report, set the struct accordingly
        if     (module == 4)  column_report = &gamepad_report_col_1;
        else if(module == 8)  column_report = &gamepad_report_col_2;
        else if(module == 12) column_report = &gamepad_report_col_3;
        else if(module == 16) column_report = &gamepad_report_col_4;
        

        //printf("Scanning protogrid for new data...\n");
        if(module_IDs[module])
        {
            // Current module is connected.

            // Process output data into out_buf
            //bool valid = master.MasterWrite(out_buf, in_buf, BUF_LEN);
            // ... TBD

            // Select the module over SPI
            master.SlaveSelect(module);

            // Read module data over SPI
            bool valid = master.MasterRead(out_buf, in_buf, BUF_LEN);

            // If read is invalid, set the module as disconnected
            if(!valid)
            {
                module_IDs[module] = DISCONNECTED;
            }
            else
            {
                connectedModules = true;
            }

            // Deselect the module
            master.SlaveSelect(NO_SLAVE_SELECTED_CSN);

            // Process module output from in_buf based on the module identifier
            switch(module_IDs[module])
            {
                case BUTTON_MODULE:
                    {
                        // If the button (active-low) is pressed, mask a bit
                        if(in_buf[0] == 0x00) {
                            column_report->digitals |= (1 << digitalCount); 
                        }

                        // Increment the number of digital inputs for this column
                        digitalCount++;

                        printf("Button: %d\n", in_buf[0]);
                    }

                    break;
                
                case JOYSTICK_MODULE:
                    {
                        // Joystick data is 12-bits
                        uint16_t x = (in_buf[1] << 8) | in_buf[0];
                        uint16_t y = (in_buf[3] << 8) | in_buf[2];

                        // Convert data into signed holders between -2048 and 2047
                        int16_t delta_x = (int16_t) (x - 2048);
                        int16_t delta_y = (int16_t) (y - 2048);

                        // Assign the joystick data to two analog axes
                        column_report->analogs[analogCount]   = delta_x;
                        column_report->analogs[analogCount+1] = delta_y;

                        // Add 2 to the number of analog inputs for this column
                        analogCount += 2;

                        //printbuf(in_buf, BUF_LEN);
                        printf("Delta X: %d\n",   delta_x);
                        printf("Delta Y: %d\n\n", delta_y);
                    }

                    break;

                default:
                    printf("Module %u appears to have disconnected or is invalid: ", module);
                    printf(module_names[module_IDs[module]]);
                    printf("\n");

                    break;
            }

            // If spi is acting up... uncomment
            //sleep_ms(10);
        }
    }

    if(connectedModules)
    {
        gpio_put(MASTER_LED_R_PIN, 1);
        gpio_put(MASTER_LED_G_PIN, 0);
    }
    else
    {
        gpio_put(MASTER_LED_R_PIN, 0);
        gpio_put(MASTER_LED_G_PIN, 1);
    }

    #if defined(DEBUG)
    if(!gpio_get(15))
    {
        gamepad_report_col_0.analogs[0] = -1000;
        gamepad_report_col_0.analogs[1] = 1000;
        gamepad_report_col_0.digitals |= 0x01;

        gamepad_report_col_1.analogs[0] = -1000;
        gamepad_report_col_1.analogs[1] = 1000;
        gamepad_report_col_1.digitals |= 0x01;

        gamepad_report_col_2.analogs[0] = -1000;
        gamepad_report_col_2.analogs[1] = 1000;
        gamepad_report_col_2.digitals |= 0x01;

        gamepad_report_col_3.analogs[0] = -1000;
        gamepad_report_col_3.analogs[1] = 1000;
        gamepad_report_col_3.digitals |= 0x01;

        gamepad_report_col_4.analogs[0] = -1000;
        gamepad_report_col_4.analogs[1] = 1000;
        gamepad_report_col_4.digitals |= 0x01;
    }
    #endif
}


//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+

int main() {
    // Sleep for module initialization and time to setup serial console
    //sleep_ms(5000);
    // NOTE: leaving this uncommented is probably why we sometimes see no connected
    // modules on reset, because we aren't giving the mod boards enough time to set up
    // and wait for master to initiate a SPI transaction.
    
    // Initialize the board
    board_init();
    // NOTE: what exactly this does could use some further investigation

    // Initialize TinyUSB
    tud_init(BOARD_TUD_RHPORT);
    // NOTE: sleeps after tusb_init() or tud_init() appear to cause the USB mount to fail.

    // Initialize standard I/O
    //stdio_init_all(); // Use w/o TinyUSB
    stdio_usb_init(); // Use w/ TinyUSB
    printf("MASTER INITIALIZATION PROCEDURE\n");
    printf("===============================\n");

    // Initialize SPI
    master.MasterInit();
    printf("[!] Master SPI Initialized\n");

    // Initialize GPIO
    init_gpio();
    printf("[!] Master GPIO Initialized\n");

    // Initial scan for modules
    rescan_modules();
    
    // Infinite task loop
    while(true) {
        tud_task();

        modules_task();

        #if defined(DEBUG)
        led_blinking_task();
        #endif

        hid_task();
    }
}