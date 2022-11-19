#include "pico/stdlib.h"
#include "../libraries/SPIMaster.h"
#include "usb_descriptors.h"
#include "tusb.h"
#include "bsp/board.h"
#include <stdlib.h>
#include <stdio.h>

enum
{
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

typedef enum
{
  NOT_CONNECTED,
  BUTTON_MODULE,
  JOYSTICK_MODULE
} moduleID_t;

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

int filter_joystick_deadzone(int coord)
{
  int in_min = 0, in_max = 65000, out_min = -5, out_max = 5;
  int result = int((coord - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
  if (abs(coord - 32768) > 500)
    return result;
  else
    return 0;
}

void printbuf(uint8_t buf[], size_t len)
{
  int i;
  for (i = 0; i < len; ++i)
  {
    if (i % 16 == 15)
      printf("%02x\n", buf[i]);
    else
      printf("%02x ", buf[i]);
  }

  // append trailing newline if there isn't one
  if (i % 16)
  {
    putchar('\n');
  }
}

int main()
{
  stdio_init_all();
  board_init();
  tusb_init();

  printf("SPI Master\n");

  SPIMaster master(
      spi_default,
      SPI_TX_PIN,
      SPI_RX_PIN,
      SPI_SCK_PIN,
      SPI_CSN_PIN);

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

  // const float conversion_factor = 3.3f / (1 << 12);
  sleep_ms(10000);
  printf("Identifying Modules...");
  gpio_put(13, 0);
  gpio_put(14, 1);

  uint8_t module1 = master.MasterIdentify();

  printf("Module 1 is Identified as %u\n", module1);
  gpio_put(13, 1);
  gpio_put(14, 1);

  gpio_put(13, 1);
  gpio_put(14, 0);

  uint8_t module2 = master.MasterIdentify();

  printf("Module 2 is Identified as %u\n", module2);
  gpio_put(13, 1);
  gpio_put(14, 1);

  while (1)
  {

    // Select the button module, read module data, and print
    // master.SlaveSelect(0);
    gpio_put(13, 0);
    gpio_put(14, 1);
    // sleep_ms(500); // DEBUG
    master.MasterRead(out_buf, in_buf, BUF_LEN);
    // master.SlaveSelect(NO_SLAVE_SELECTED);
    gpio_put(13, 1);
    gpio_put(14, 1);
    printf("BUTTON PACKET\n");
    printbuf(in_buf, BUF_LEN);
    printf("\n");

    // sleep_ms(500);

    // Select the joystick module, read module data, and print
    // master.SlaveSelect(1);
    gpio_put(13, 1);
    gpio_put(14, 0);
    // sleep_ms(500); // DEBUG
    master.MasterRead(out_buf, in_buf, BUF_LEN);
    // master.SlaveSelect(NO_SLAVE_SELECTED);
    gpio_put(13, 1);
    gpio_put(14, 1);
    printf("JOYSTICK PACKET\n");
    printbuf(in_buf, BUF_LEN);
    printf("\n");

    tud_task(); // tinyusb device task
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

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if (!tud_hid_ready())
    return;

  switch (report_id)
  {
  case REPORT_ID_KEYBOARD:
  {
    // use to avoid send multiple consecutive zero report for keyboard
    static bool has_keyboard_key = false;

    if (btn)
    {
      uint8_t keycode[6] = {0};
      keycode[0] = HID_KEY_A;

      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
      has_keyboard_key = true;
    }
    else
    {
      // send empty key report if previously has key pressed
      if (has_keyboard_key)
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
      has_keyboard_key = false;
    }
  }
  break;

  case REPORT_ID_MOUSE:
  {
    int8_t const delta = 5;

    // no button, right + down, no scroll, no pan
    tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
  }
  break;

  case REPORT_ID_CONSUMER_CONTROL:
  {
    // use to avoid send multiple consecutive zero report
    static bool has_consumer_key = false;

    if (btn)
    {
      // volume down
      uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
      tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
      has_consumer_key = true;
    }
    else
    {
      // send empty key report (release key) if previously has key pressed
      uint16_t empty_key = 0;
      if (has_consumer_key)
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
      has_consumer_key = false;
    }
  }
  break;

  case REPORT_ID_GAMEPAD:
  {
    // use to avoid send multiple consecutive zero report for keyboard
    static bool has_gamepad_key = false;

    hid_gamepad_report_t report =
        {
            .x = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0, .hat = 0, .buttons = 0};

    if (btn)
    {
      report.hat = GAMEPAD_HAT_UP;
      report.buttons = GAMEPAD_BUTTON_A;
      tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

      has_gamepad_key = true;
    }
    else
    {
      report.hat = GAMEPAD_HAT_CENTERED;
      report.buttons = 0;
      if (has_gamepad_key)
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
      has_gamepad_key = false;
    }
  }
  break;

  default:
    break;
  }
}

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

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if (tud_suspended() && btn)
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }
  else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, btn);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
  (void)instance;
  (void)len;

  uint8_t next_report_id = report[0] + 1;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
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
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void)instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if (bufsize < 1)
        return;

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
  if (!blink_interval_ms)
    return;

  // Blink every interval ms
  if (board_millis() - start_ms < blink_interval_ms)
    return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
