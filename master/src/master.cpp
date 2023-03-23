#include "pico/stdlib.h"
#include "pico/stdio.h"
//#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "hardware/adc.h"
#include "../libraries/SPIMaster.h"
#include "usb_descriptors.h"
#include "tusb.h"
#include "bsp/board.h"
#include "../../prototroller.h"
#include "../../commons.h"

// Uncomment to run in debug mode
//#define DEBUG 0

//--------------------------------------------------------------------+
// Types and Enums
//--------------------------------------------------------------------+

// Enum for times
enum
{
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
    PRINT_REPORT_DELAY = 2000,
};

// Prototroller HID Gamepad Report
// Must support the maximum number of each input module per column
typedef struct
{
    uint16_t    digitals;   // Digital inputs (momentary button, maintained button, D-pad, XYAB, etc.)
    int16_t     analogs[8]; // Analog inputs (joystick, slider, twist switch, etc.)
} gamepad_report_t;

// Prototroller CDC Commands
// Commands sent over CDC (Serial) from the host
// Must support the maximum number of output modules overall
typedef struct
{
    uint32_t    digitals;
    int16_t     analogs[20];
} cdc_commands_t;


//--------------------------------------------------------------------+
// Globals
//--------------------------------------------------------------------+

// Gamepad reports for each column
gamepad_report_t gamepad_report_col_0;
gamepad_report_t gamepad_report_col_1;
gamepad_report_t gamepad_report_col_2;
gamepad_report_t gamepad_report_col_3;
gamepad_report_t gamepad_report_col_4;

// CDC Command Data Store
cdc_commands_t commands;

// SPI Master
SPIMaster master(
    spi_default,
    MASTER_SPI_TX_PIN,
    MASTER_SPI_RX_PIN,
    MASTER_SPI_SCK_PIN,
    MASTER_SPI_CSN_PIN,
    true
);

// SPI Transaction Buffers
uint8_t out_buf[BUF_LEN];
uint8_t in_buf[BUF_LEN];

// Keep track of error counts
uint8_t error_counts[MAX_MODULES] = {0};

// Module Identification Data Store
moduleID_t module_IDs[MAX_MODULES] = {DISCONNECTED};

// Rescan ISR flag
volatile bool rescan = false;

// Rescan debounce timer
volatile unsigned long time = to_ms_since_boot(get_absolute_time());

// Rescan debounce delay (in ms)
const int delay = 50;

// Interval in ms for LED blinking task
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

// Interval in ms for report message task
static uint32_t report_message_interval_ms = PRINT_REPORT_DELAY;

//--------------------------------------------------------------------+
// Prototypes
//--------------------------------------------------------------------+

void print_buf(uint8_t buf[], size_t len);
void print_report(uint8_t report_id, gamepad_report_t *report);
void rescan_modules();
void init_gpio();
uint8_t assign_analog_data(gamepad_report_t *column_report, uint8_t analog_count, int16_t *data, uint8_t len);
void send_hid_report(uint8_t report_id);
void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);
void tud_resume_cb(void);
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len);
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts);
void tud_cdc_rx_cb(uint8_t itf);
void rescan_cb(uint gpio, uint32_t events);
void hid_task(void);
void cdc_task(void);
void led_blinking_task(void);
void print_report_task(void);
void modules_task(void);

//--------------------------------------------------------------------+
// Helpers
//--------------------------------------------------------------------+

// Outputs contents of a buffer to standard output.
void print_buf(uint8_t buf[], size_t len)
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

// Outputs contents of a gamepad report to standard output.
void print_report(uint8_t report_id, gamepad_report_t *report)
{
    printf("===========================\n");
    printf("GAMEPAD REPORT FOR COLUMN %01d\n", report_id);
    printf("===========================\n");

    printf("DIGITALS       : 0x%04x (%u)\n", report->digitals, report->digitals);

    printf("ANALOGS[0..7]  ");
    for(unsigned int i = 0; i < (sizeof(report->analogs) / sizeof(report->analogs[0])); i++)
    {
        printf(": 0x%04x (%d)", (unsigned int)(uint16_t)report->analogs[i], report->analogs[i]);
    }

    printf("\n\n");
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
    for(uint8_t pin = MASTER_EONA_PIN; pin <= MASTER_A0_PIN; pin++)
    {
        gpio_init(pin);
        gpio_set_dir(pin, true);
        gpio_set_pulls(pin, false, false);
        gpio_put(pin, 1);
    }
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

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    gpio_init(20);
    gpio_set_dir(20, true);
    gpio_set_pulls(20, false, false);
    gpio_put(20, true);

    gpio_init(21);
    gpio_set_dir(21, true);
    gpio_set_pulls(21, false, false);
    gpio_put(21, true);

    gpio_init(22);
    gpio_set_dir(22, true);
    gpio_set_pulls(22, false, false);
    gpio_put(22, true);
    #endif
}

// Assign analog data to the specified report, clamping at the maximum allowed axes
uint8_t assign_analog_data(gamepad_report_t *column_report, uint8_t analog_count, int16_t *data, uint8_t len)
{
    // Calculate the maximum number of axes
    uint8_t max = sizeof(column_report->analogs) / sizeof(column_report->analogs[0]);

    // If the total number of axes will surpass the limit, then clamp it
    if((analog_count + len) > max)
    {
        len = max - analog_count;
    }

    // Assign the data
    for(uint8_t i = 0; i < len; i++)
    {
        column_report->analogs[analog_count+i] = data[i];
    }
    
    // Return the new analog count
    return analog_count + len;
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
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) rts;

    if ( dtr )
    {
        // Terminal connected
    }
    else
    {
        // Terminal disconnected
    }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
    (void) itf;
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

// Task to process CDC data
void cdc_task(void)
{
    // The serial interface receiving commands from host
    uint8_t itf_rx = 1;

    // The serial interface transmitting output to host
    uint8_t itf_tx = 0;

    if( tud_cdc_n_connected(itf_rx) )
    {
        // Connected
        if( tud_cdc_n_available(itf_rx) )
        {
            // Data available
            char buf[64];

            // Tokenized data
            char tokens[3][64];

            // Read
            uint32_t count = tud_cdc_n_read(itf_rx, buf, sizeof(buf));

            // Tokenize
            char *token;
            token = strtok(buf, " ");
            uint8_t index = 0;
            
            // Extract 3 tokens at most from the buffer
            while(token != NULL && index < 3)
            {
                strcpy(tokens[index++], token);
                token = strtok(NULL, " ");
            }

            if(strcmp(tokens[0], "digital") == 0)
            {
                int module = atoi(tokens[1]);
                int value = atoi(tokens[2]);
                if(module >= 0 && module <= 19)
                {
                    if(value == 0)
                    {
                        commands.digitals &= ~(1 << module);
                    }
                    else if(value == 1)
                    {
                        commands.digitals |= (1 << module);
                    }
                }
            }
            else if(strcmp(tokens[0], "analog") == 0)
            {
                int module = atoi(tokens[1]);
                int value = atoi(tokens[2]);

                if(module >= 0 && module <= 19)
                {
                    commands.analogs[module] = value;
                }
            }
            else if(strcmp(tokens[0], "resetd") == 0)
            {
                commands.digitals = 0;
            }
            else if(strcmp(tokens[0], "reseta") == 0)
            {
                memset(commands.analogs, 0, sizeof(commands.analogs));
            }
            else if(strcmp(tokens[0], "reset") == 0)
            {
                memset(&commands, 0, sizeof(cdc_commands_t));
            }

            // Echo back
            //tud_cdc_n_write(itf_tx, buf, count);
 
            // Flush
            //tud_cdc_n_write_flush(itf_tx);

        }
    }
}

// Task to blink an LED
void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // blink is disabled
    if (!blink_interval_ms) return;

    // Blink every blink_interval_ms ms
    if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time

    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state;
}

// Task to routinely print gamepad reports
void print_report_task(void)
{
    static uint32_t start_ms = 0;

    // Printing reports is disabled
    if (!report_message_interval_ms) return;

    // Print a report every report_message_interval_ms ms
    if ( board_millis() - start_ms < report_message_interval_ms) return; // not enough time

    start_ms += report_message_interval_ms;

    print_report(REPORT_ID_COLUMN_0, &gamepad_report_col_0);
    print_report(REPORT_ID_COLUMN_1, &gamepad_report_col_1);
    print_report(REPORT_ID_COLUMN_2, &gamepad_report_col_2);
    print_report(REPORT_ID_COLUMN_3, &gamepad_report_col_3);
    print_report(REPORT_ID_COLUMN_4, &gamepad_report_col_4);
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
    uint8_t digital_count = 0;
    uint8_t analog_count = 0;

    // The gamepad report for the current column
    gamepad_report_t *column_report = &gamepad_report_col_0;

    for(uint8_t module = 0; module < MAX_MODULES; module++)
    {
        // Reset digital and analog count for each column report
        if(module % 4 == 0)
        {
            digital_count = 0;
            analog_count  = 0;
        }

        // If this is the beginning of a new column report, set the struct accordingly
        if     (module == 4)  column_report = &gamepad_report_col_1;
        else if(module == 8)  column_report = &gamepad_report_col_2;
        else if(module == 12) column_report = &gamepad_report_col_3;
        else if(module == 16) column_report = &gamepad_report_col_4;
        
        // If this module is connected, exchange data over SPI and process into column report
        if(module_IDs[module])
        {
            // Process output data into out_buf based on the modile identifier
            switch(module_IDs[module])
            {
                // --- DIGITAL OUTPUTS
                case LED:
                    {
                        // Forward data from digital commands to the LED module
                        out_buf[0] = 0x01 & (uint8_t) (commands.digitals >> module);
                    }
                    break;

                default:
                    break;
            }

            // Select the module over SPI
            master.SlaveSelect(module);

            // Read module data over SPI
            bool valid = master.MasterReadWrite(out_buf, in_buf, BUF_LEN);

            // If read is invalid, set the module as disconnected
            if(!valid)
            {
                module_IDs[module] = DISCONNECTED;
                printf("Module %u appears to have disconnected or is invalid: ", module);
                printf(module_names[module_IDs[module]]);
                printf("\n");
            }
            else
            {
                connectedModules = true;
            }

            // Deselect the module
            master.SlaveSelect(NO_SLAVE_SELECTED_CSN);

            // Process module I/O based on the module identifier
            switch(module_IDs[module])
            {
                // --- DIGITAL INPUTS ---
                case BUTTON_MAINTAINED:
                case BUTTON_MOMENTARY:
                    {
                        // If the button (active-low) is pressed, mask a bit
                        column_report->digitals |= ((in_buf[0]==0x00) << digital_count); 

                        // Increment the number of digital inputs for this column
                        digital_count++;
                    }

                    break;

                case XYAB:
                case DPAD:
                    {
                        // Mask any bits
                        column_report->digitals |= ((in_buf[0]==0x00) << digital_count); 
                        column_report->digitals |= ((in_buf[1]==0x00) << (digital_count+1)); 
                        column_report->digitals |= ((in_buf[2]==0x00) << (digital_count+2)); 
                        column_report->digitals |= ((in_buf[3]==0x00) << (digital_count+3)); 

                        // Increment the number of digital inputs for this column
                        digital_count += 4;
                    }
                    break;
    

                // --- ANALOG INPUTS ---
                case JOYSTICK:
                    {
                        // Joystick data is 12-bits
                        uint16_t x = (in_buf[1] << 8) | in_buf[0];
                        uint16_t y = (in_buf[3] << 8) | in_buf[2];

                        // Convert data into signed holders between -2048 and 2047
                        int16_t delta_x = (int16_t) (x - 2048);
                        int16_t delta_y = (int16_t) (y - 2048);

                        // Assign the joystick data to two analog axes, if space is available
                        int16_t joystick_data[2] = { delta_x, delta_y };
                        analog_count = assign_analog_data(column_report, analog_count, joystick_data, 2);
                    }

                    break;

                case SLIDER:
                case TWIST_SWITCH:
                    {
                        // Potentiometer data is 12-bits
                        uint16_t wiper = (in_buf[1] << 8) | in_buf[0];

                        // Convert data into a signed holder between -2048 and 2047
                        int16_t delta_wiper = (int16_t) (wiper - 2048);

                        // Assign the potentiometer data to one analog axis, if space is available
                        analog_count = assign_analog_data(column_report, analog_count, &delta_wiper, 1);
                    }
                    break;

                case ACCEL:
                    {
                        // Due to 8-axis limit of DirectInput, cannot support a full column of 3/4 accel/gyro (9/12 axes)
                        // Something like joystick + twist switch + accel + gyro would not work (9 axes)
                        // However, 2x twist switch + accel + gyro would work (8 axes)
                        // Or, a slider (1x2) + accel + gyro (7 axes)
                    }
                    break;

                case GYRO:
                    {
                        // Due to 8-axis limit of DirectInput, cannot support a full column of 3/4 accel/gyro (9/12 axes)
                        // Something like joystick + twist switch + accel + gyro would not work (9 axes)
                        // However, 2x twist switch + accel + gyro would work (8 axes)
                        // Or, a slider (1x2) + accel + gyro (7 axes)
                    }
                    break;

                default:
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
    // Mock gamepad reports for each column

    gamepad_report_col_0.analogs[0] = (int16_t) (adc_read() - 2048);
    gamepad_report_col_1.analogs[0] = (int16_t) (adc_read() - 2048);
    gamepad_report_col_2.analogs[0] = (int16_t) (adc_read() - 2048);
    gamepad_report_col_3.analogs[0] = (int16_t) (adc_read() - 2048);
    gamepad_report_col_4.analogs[0] = (int16_t) (adc_read() - 2048);

    if(!gpio_get(15))
    {
        gamepad_report_col_0.digitals |= 0x01;
        gamepad_report_col_1.digitals |= 0x01;
        gamepad_report_col_2.digitals |= 0x01;
        gamepad_report_col_3.digitals |= 0x01;
        gamepad_report_col_4.digitals |= 0x01;
    }

    // Mock LED outputs
    gpio_put(20, (0x00000001 & commands.digitals));
    gpio_put(21, (0x00000002 & commands.digitals));
    gpio_put(22, (0x00000004 & commands.digitals));
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
    //rescan_modules();
    
    // Infinite task loop
    while(true) {
        tud_task();

        modules_task();

        #if defined(DEBUG)
        led_blinking_task();
        #endif

        print_report_task();

        hid_task();
        
        cdc_task();
    }
}