#ifndef _BOARDS_PROTOTROLLER_H
#define _BOARDS_PROTOTROLLER_H

// For board detection
#define PROTOTROLLER



/* CDC COMMANDS FROM HOST
 * digital [0/1/.../19] [0/1]
 *   for example: digital 12 1 will output digital 1 to module 12, if connected
 * analog [0/1/.../19] [-2048/-2047/.../2047]
 *   for example: analog 3 256 will output ~0.206V to module 3, if connected
 *   (component must have internal DAC or use DAC on interface board)
 * resetd
 *   reset only the digital outputs
 * reseta
 *   reset only the analog outputs
 * reset
 *   reset everything
 */  



// --- MASTER BOARD BEGIN ---
// LED
#define MASTER_LED_R_PIN 2
#define MASTER_LED_G_PIN 3
#define MASTER_LED_B_PIN 4

// CSN SELECTS
#define MASTER_CSN_END_PIN     6 // For D2M debugging
#define MASTER_CSN_START_PIN   5 // For D2M debugging
#define MASTER_A0_PIN 12
#define MASTER_A1_PIN 11
#define MASTER_A2_PIN 10
#define MASTER_A3_PIN 9
#define MASTER_EONB_PIN 8
#define MASTER_EONA_PIN 7

// SPI
#define MASTER_SPI_TX_PIN      19
#define MASTER_SPI_SCK_PIN     18
#define MASTER_SPI_CSN_PIN     5 //17?
#define MASTER_SPI_RX_PIN      16

// MISC
#define MASTER_RESCAN_PIN 17
#define MASTER_RETRY_ON_ERROR_COUNT 20
// --- MASTER BOARD END ---



// --- MODULE BOARD BEGIN ---
// SPI
#define MODULE_SPI_TX_PIN      19
#define MODULE_SPI_SCK_PIN     18
#define MODULE_SPI_CSN_PIN     17
#define MODULE_SPI_RX_PIN      16

// BUTTON MODULE
#define MODULE_BUTTON_PIN 27

// MAINTAINED BUTTON MODULE
#define MODULE_MAINTAINED_BUTTON_PIN 26

// XYAB MODULE
#define MODULE_XYAB_PIN1 25
#define MODULE_XYAB_PIN2 26
#define MODULE_XYAB_PIN3 27
#define MODULE_XYAB_PIN4 28

// DPAD MODULE
#define MODULE_DPAD_PIN1 25
#define MODULE_DPAD_PIN2 26
#define MODULE_DPAD_PIN3 27
#define MODULE_DPAD_PIN4 28

// JOYSTICK MODULE
#define MODULE_JOYSTICK_VRX_PIN 27
#define MODULE_JOYSTICK_VRY_PIN 26

// SLIDER MODULE
#define MODULE_SLIDER_ADCPIN 26

// TWIST SWITCH MODULE
#define MODULE_TWIST_SWITCH_ADCPIN 27

// LED MODULE
#define MODULE_LED_PIN 25

// --- MODULE BOARD END ---


// --- UART ---
#ifndef PICO_DEFAULT_UART
#define PICO_DEFAULT_UART 0
#endif
#ifndef PICO_DEFAULT_UART_TX_PIN
#define PICO_DEFAULT_UART_TX_PIN 0
#endif
#ifndef PICO_DEFAULT_UART_RX_PIN
#define PICO_DEFAULT_UART_RX_PIN 1
#endif

// --- LED ---
#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN MASTER_LED_R_PIN
#endif
#ifndef PICO_DEFAULT_LED_PIN_INVERTED
#define PICO_DEFAULT_LED_PIN_INVERTED 1
#endif

// --- I2C ---
#ifndef PICO_DEFAULT_I2C
#define PICO_DEFAULT_I2C 1
#endif
#ifndef PICO_DEFAULT_I2C_SDA_PIN
#define PICO_DEFAULT_I2C_SDA_PIN 2
#endif
#ifndef PICO_DEFAULT_I2C_SCL_PIN
#define PICO_DEFAULT_I2C_SCL_PIN 3
#endif

// --- SPI ---
#define BAUD_RATE   12000*1000 // 12 Mbps baud rate
#define BUF_LEN     0x100 // 256(257?)-byte buffer
#ifndef PICO_DEFAULT_SPI
#define PICO_DEFAULT_SPI 0
#endif
#ifndef PICO_DEFAULT_SPI_SCK_PIN
#define PICO_DEFAULT_SPI_SCK_PIN 18
#endif
#ifndef PICO_DEFAULT_SPI_TX_PIN
#define PICO_DEFAULT_SPI_TX_PIN 19
#endif
#ifndef PICO_DEFAULT_SPI_RX_PIN
#define PICO_DEFAULT_SPI_RX_PIN 16
#endif
#ifndef PICO_DEFAULT_SPI_CSN_PIN
#define PICO_DEFAULT_SPI_CSN_PIN 5
#endif

// --- FLASH ---
#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1
#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 2
#endif
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)
#endif

// All boards have B1 RP2040
#ifndef PICO_RP2040_B0_SUPPORTED
#define PICO_RP2040_B0_SUPPORTED 0
#endif

#endif