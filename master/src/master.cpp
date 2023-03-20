#include "pico/stdlib.h"
#include "pico/stdio.h"
//#include <stdio.h>
#include <stdlib.h>
#include "../libraries/SPIMaster.h"
#include "usb_descriptors.h"
#include "tusb.h"
#include "bsp/board.h"
#include "../../prototroller.h"

#define MOUSE_SENS 5
#define NUM_JOYSTICKS 3
/* Enumerations */
enum
{
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

typedef enum {
    DISCONNECTED,
    BUTTON_MODULE,
    JOYSTICK_MODULE
} moduleID_t;

const char module_names[][20] =
    {
        "DISCONNECTED",
        "BUTTON MODULE",
        "JOYSTICK MODULE"
    };

/* Rescan debouncing */
volatile bool rescan = false;
volatile unsigned long time = to_ms_since_boot(get_absolute_time());
const int delay = 50; // Rescan button debounce delay

/* TinyUSB mounted blink interval */
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

/* Current Module Counts for data formatting */
uint8_t numButtons = 0;
uint8_t numJoysticks = 0;

/* Prototroller HID Gamepad Report
   Must support the maximum number of each input module; for a 4x5 grid:
   20 1x1 Button (Momentary AND Maintained)
   20 1x1 Joystick
   20 1x1 Potentiometer [TODO]
   10 1x2 Slider [TODO]
   04 2x2 D-pad [TODO]
   04 2x2 XYAB-pad [TODO]
*/
typedef struct
{
    //uint8_t     report_id;  // Report ID, TinyUSB already accounts for this
    uint16_t    buttons;   // Masks for currently pressed buttons
    int8_t      analog[8];
} gamepad_report_t;

gamepad_report_t gamepad_report_col_0;
gamepad_report_t gamepad_report_col_1;
gamepad_report_t gamepad_report_col_2;
gamepad_report_t gamepad_report_col_3;
gamepad_report_t gamepad_report_col_4;

volatile bool reportSent = false;

/* Prototypes */
void led_blinking_task(void);
void hid_task(void);

/* SPI Master */
SPIMaster master(
        spi_default,
        MASTER_SPI_TX_PIN,
        MASTER_SPI_RX_PIN,
        MASTER_SPI_SCK_PIN,
        MASTER_SPI_CSN_PIN,
        false
    );

/* Buffers */
uint8_t out_buf[BUF_LEN];
uint8_t in_buf[BUF_LEN];

/* Module state data store */
moduleID_t module_IDs[MAX_MODULES] = {DISCONNECTED};


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
 * Update the module identification to disconnected
 * or a unique module specifier.
 */
void rescan_modules()
{
    printf("[!] Rescanning for active modules...\n");
    numButtons = 0;
    numJoysticks = 0;
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

        // switch (identification)
        // {
        // case BUTTON_MODULE:
        //     numButtons += 1;
        //     break;
        
        // case JOYSTICK_MODULE:
        //     numJoysticks +=1;
        // default:
        //     break;
        // }

        char status = (identification == DISCONNECTED) ? 'x' : '!';

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
    for(uint8_t pin = MASTER_CSN_START_PIN; pin <= MASTER_CSN_END_PIN; pin++)
    {
        gpio_init(pin);
        gpio_set_dir(pin, true);
        gpio_set_pulls(pin, false, false);
        gpio_put(pin, 1);
    }

    // Initialize GPIO pin for rescan button
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

    //Setup the rescan callback
    gpio_set_irq_enabled_with_callback(
        MASTER_RESCAN_PIN,
        GPIO_IRQ_EDGE_RISE,
        true,
        &rescan_callback
    );

    // Debug past here.
    gpio_init(15);
    gpio_set_dir(15, false);
    gpio_set_pulls(15, false, false);
}

void modules_task()
{
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
    uint8_t buttonIndex = 0;
    uint8_t analogIndex = 0;

    for(uint8_t module = 0; module < MAX_MODULES; module++)
    {
        gamepad_report_t *column_report;

        if      (module < 4) column_report = &gamepad_report_col_0;
        else if (module < 8) column_report = &gamepad_report_col_1;
        else if(module < 12) column_report = &gamepad_report_col_2;
        else if(module < 16) column_report = &gamepad_report_col_3;
        else if(module < 20) column_report = &gamepad_report_col_4;
        
        //printf("Scanning protogrid for new data...\n");
        if(module_IDs[module])
        {
            // Current module is connected.

            // Process module input into out_buf
            // ... TBD

            // Select the module
            master.SlaveSelect(module);

            // Read module data
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

            // Process module output from in_buf
            switch(module_IDs[module])
            {
                case BUTTON_MODULE:
                    {
                        // If the button (active-low) is pressed, mask a bit
                        if(in_buf[0] == 0x00) {
                            column_report->buttons |= (1 << buttonIndex); 
                        }

                        // Increment the button index
                        buttonIndex++;

                        printf("Button: %d\n", in_buf[0]);
                    }

                    break;
                
                case JOYSTICK_MODULE:
                    {
                        // Convert the joystick data
                        uint16_t x = (in_buf[1] << 8) | in_buf[0];
                        uint16_t y = (in_buf[3] << 8) | in_buf[2];
                        uint16_t offset = 200;
                        int8_t delta_x = ((x + offset) >> 9)-4;
                        int8_t delta_y = ((y + offset) >> 9)-4;

                        // switch(joystickIndex)
                        // {
                        //     case 0:
                        //         //gamepad_report.x1 = delta_x;
                        //         //gamepad_report.y1 = delta_x
                        //         //gamepad_report.z1 = 0;
                        //         break;
                            
                        //     case 1:
                        //         //gamepad_report.x2 = delta_x;
                        //         //gamepad_report.y2 = delta_y;
                        //         //gamepad_report.z2 = 0;
                        //         break;

                        //     default:
                        //         // Cannot support more than 2 joysticks atm (hard gamepad report limits)
                        //         // BUT: could assign joysticks after the 2nd to other things, such as
                        //         // mouse, keyboard, generic I/O, etc.
                        //         // OR: Investigate sending multicollections (would need 10 for 2 joysticks each)
                        //         break;
                        // }

                        column_report->analog[analogIndex]   = delta_x;
                        column_report->analog[analogIndex+1] = delta_y;

                        // Increment the analog index by 2 (two joystic axes)
                        analogIndex += 2;

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

    // Debug only past here.
    if(!gpio_get(15))
    {
        gamepad_report_col_0.analog[0] += 25;
        gamepad_report_col_0.analog[1] += 25;
        gamepad_report_col_0.buttons |= 0x01;

        gamepad_report_col_1.analog[2] += 25;
        gamepad_report_col_1.analog[3] += 25;
        gamepad_report_col_1.buttons |= 0x01;

        gamepad_report_col_2.analog[4] += 25;
        gamepad_report_col_2.analog[5] += 25;
        gamepad_report_col_2.buttons |= 0x01;

        gamepad_report_col_3.analog[6] += 25;
        gamepad_report_col_3.analog[7] += 25;
        gamepad_report_col_3.buttons |= 0x01;

        gamepad_report_col_4.analog[0] += 25;
        gamepad_report_col_4.analog[1] += 25;
        gamepad_report_col_4.buttons |= 0x01;
    }
}

int main() {
    // Sleep for module initialization and time to setup console
    //sleep_ms(5000);
    
    board_init();
    tud_init(BOARD_TUD_RHPORT);

    //stdio_init_all(); // Use w/o TinyUSB
    //stdio_usb_init(); // Use w/ TinyUSB

    // NOTE: sleeps after tusb_init() appear to cause the USB mount to fail.
    //sleep_ms(5000);

    //printf("MASTER INITIALIZATION PROCEDURE\n");
    //printf("===============================\n");

    // Initialize SPI Stuff
    master.MasterInit();

    //printf("[!] Master SPI Initialized\n");

    // Initialize all GPIO
    init_gpio();

    //printf("[!] Master GPIO Initialized\n");

    // Initial rescan
    rescan_modules();
    
    while(true) {
        tud_task();
        modules_task();
        led_blinking_task();
        hid_task();
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

// Send for each applicable HID profile (gamepad, keyboard, mouse, etc ..)
// tud_hid_report_complete_cb() is used when the report has completed sending to host
//void hid_task(void)
//{
//     // Remote wakeup (comment out?)
//     if (tud_suspended())
//     {
//         // Wake up host if we are in suspend mode
//         // and REMOTE_WAKEUP feature is enabled by host
//         tud_remote_wakeup();
//     }
//     // else
//     // {
//     //     // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
//     //     send_hid_report(REPORT_ID_MOUSE, 0);
//     // }

//     // Skip sending the reports if HID is not ready yet
//     if (tud_hid_ready()) {
//         // Format and send HID report data
//         // switch (numJoysticks) {
//         //     //Populate Analog Components Properly
//         //     case 2:
//         //         report.z = joystickDeltas[1][0];
//         //         report.rz = joystickDeltas[1][1];
//         //         //TODO -- Additional Analog Components (Dials/Sliders) Logic for population (as we have upper limit of 8 analog components on a generic gamepad packet with picoSDK declaration)
//         //         //Intentional No Break
//         //     case 1:
//         //         report.x = joystickDeltas[0][0];
//         //         report.y = joystickDeltas[0][1];
//         //         break;
//         //     default: break;
//         // }
//         //report.rx = 0;
//         //report.ry = 0;
//         //report.hat = 0; //TODO : D-Pad Values 
//         //Add button data
//         //report.buttons = buttons;
//         // Mouse (from joystick and buttons)
//         //tud_hid_mouse_report(REPORT_ID_MOUSE, buttons, delta_x, delta_y, 0, 0);

//         //Generic Gamepad
//         //tud_hid_report(REPORT_ID_GAMEPAD, &gamepad_report, sizeof(gamepad_report));
//         tud_hid_n_report(0, REPORT_ID_GAMEPAD, &gamepad_report, sizeof(gamepad_report));
//     }
// }

static void send_hid_report(uint8_t report_id)
    {
        // skip if hid is not ready yet
        if ( !tud_hid_ready() ) return;

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

void hid_task(void)
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if ( board_millis() - start_ms < interval_ms) return; // not enough time
    start_ms += interval_ms;

    // Remote wakeup
    if ( tud_suspended() )
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }
    else
    {
        send_hid_report(REPORT_ID_COLUMN_0); // Kick-off the report chain here
        //tud_hid_n_report(0x00, REPORT_ID_COLUMN_0, &gamepad_report_col_0, sizeof(gamepad_report_col_0));
        //tud_hid_n_report(0x00, REPORT_ID_COLUMN_1, &gamepad_report_col_1, sizeof(gamepad_report_col_1));
        //tud_hid_n_report(0x00, REPORT_ID_COLUMN_2, &gamepad_report_col_2, sizeof(gamepad_report_col_2));
        //tud_hid_n_report(0x00, REPORT_ID_COLUMN_3, &gamepad_report_col_3, sizeof(gamepad_report_col_3));
        //tud_hid_n_report(0x00, REPORT_ID_COLUMN_4, &gamepad_report_col_4, sizeof(gamepad_report_col_4));
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
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

    // if (report_type == HID_REPORT_TYPE_OUTPUT)
    // {
    //     // Set keyboard LED e.g Capslock, Numlock etc...
    //     if (report_id == REPORT_ID_KEYBOARD)
    //     {
    //         // bufsize should be (at least) 1
    //         if ( bufsize < 1 ) return;

    //         uint8_t const kbd_leds = buffer[0];

    //         if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
    //         {
    //             // Capslock On: disable blink, turn led on
    //             blink_interval_ms = 0;
    //             board_led_write(true);
    //         }
    //         else
    //         {
    //             // Caplocks Off: back to normal blink
    //             board_led_write(false);
    //             blink_interval_ms = BLINK_MOUNTED;
    //         }
    //     }
    // }
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