#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"
#include <math.h>

#include "usb_descriptors.h"
#include "hardware/gpio.h"

#define PI 3.14

#define BUTTON_LEFT 10       // connected to a pull up resistor
#define BUTTON_DOWN 7       // connected to a pull up resistor
#define BUTTON_UP 8         // connected to a pull up resistor
#define BUTTON_RIGHT 19     // connected to a pull up resistor
#define BUTTON_MODE 28      // connected to a pull up resistor

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);
void button_initialize();

int main(void)
{
  board_init();
  tud_init(BOARD_TUD_RHPORT);
  button_initialize();

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  while (1)
  {
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
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

/*
0: Regular mode
1: Remote working mode
*/

void button_initialize(){
  gpio_init(BUTTON_UP);
  gpio_init(BUTTON_RIGHT);
  gpio_init(BUTTON_DOWN);
  gpio_init(BUTTON_LEFT);
  gpio_init(BUTTON_MODE);

  gpio_set_dir(BUTTON_UP, false);
  gpio_set_dir(BUTTON_RIGHT, false);
  gpio_set_dir(BUTTON_DOWN, false);
  gpio_set_dir(BUTTON_LEFT, false);
  gpio_set_dir(BUTTON_MODE, false);
}

uint8_t mouse_mode(){
  int mode=0; // if nothing pressed mode = 0

  if (!gpio_get(BUTTON_UP) && !gpio_get(BUTTON_LEFT)) {
    mode = 5;
  }
  else if (!gpio_get(BUTTON_UP) && !gpio_get(BUTTON_RIGHT)) {
    mode = 6;
  }
  else if (!gpio_get(BUTTON_RIGHT) && !gpio_get(BUTTON_DOWN)) {
    mode = 7;
  }
  else if (!gpio_get(BUTTON_LEFT) && !gpio_get(BUTTON_DOWN)) {
    mode = 8;
  }
  else if (!gpio_get(BUTTON_UP)) {
    mode = 1;
  }
  else if (!gpio_get(BUTTON_RIGHT)) {
    mode = 2;
  }
  else if (!gpio_get(BUTTON_DOWN)) {
    mode = 3;
  }
  else if (!gpio_get(BUTTON_LEFT)) {
    mode = 4;
  }
  
  return mode;
}

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      int8_t const delta = 5;
      static float angle = 0.0;


      //  printf("Mode = %d\r\n", mode);      
      
      // tud_hid_mouse_report(uint8_t report_id, uint8_t buttons, int8_t x, int8_t y, int8_t vertical, int8_t horizontal)
      //  tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, -5, -5, 0, 0);

      if (!gpio_get(BUTTON_MODE))
      {
        // DEVICE IN REMOTE WORKING MODE
        angle += 0.1;
        if (angle >= 2 * PI) angle -= 2 * PI;

        int8_t dx = (int8_t)(cos(angle) * 10);  
        int8_t dy = (int8_t)(sin(angle) * 10);

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, dx, dy, 0, 0);
        
        //  tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, -5, -5, 0, 0);
      }

      else
      {
        // DEVICE IN REGULAR MODE

        uint8_t mode = mouse_mode();
      /*
      mode          function
      0     no button pressed
      1     up
      2     right
      3     down
      4     left
      5     up and left
      6     up and right
      7     down and right
      8     down and left    
      */

      static int start_t = 0;
      static int prev_mode = 0;

      if (mode != prev_mode){
        start_t = 0;
        prev_mode = mode;
      }

      if (mode == 1){
        if (start_t == 0) {
          start_t = board_millis();  
        }
        int elapsed = board_millis() - start_t;
        int mult = 1 + (elapsed / 1000);  
        
        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, 0, -3*mult, 0, 0);
      }
      else if (mode == 2)
      {
        if (start_t == 0) {
          start_t = board_millis();  
        }
        int elapsed = board_millis() - start_t;
        int mult = 1 + (elapsed / 1000);  

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, 3*mult, 0, 0, 0);
      }
      else if (mode == 3)
      {
        if (start_t == 0) {
          start_t = board_millis();  
        }
        int elapsed = board_millis() - start_t;
        int mult = 1 + (elapsed / 1000);  

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, 0, 3*mult, 0, 0);
      }
      else if (mode == 4)
      {
        if (start_t == 0) {
          start_t = board_millis();  
        }
        int elapsed = board_millis() - start_t;
        int mult = 1 + (elapsed / 1000);  

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, -3*mult, 0, 0, 0);
      }
      else if (mode == 5)
      {
        if (start_t == 0) {
          start_t = board_millis();  
        }
        int elapsed = board_millis() - start_t;
        int mult = 1 + (elapsed / 1000);  

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, -3*mult, -3*mult, 0, 0);
      }
      else if (mode == 6)
      {
        if (start_t == 0) {
          start_t = board_millis();  
        }
        int elapsed = board_millis() - start_t;
        int mult = 1 + (elapsed / 1000);  

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, 3*mult, -3*mult, 0, 0);
      }
      else if (mode == 7)
      {
        if (start_t == 0) {
          start_t = board_millis();  
        }
        int elapsed = board_millis() - start_t;
        int mult = 1 + (elapsed / 1000);  

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, 3*mult, 3*mult, 0, 0);
      }
      else if (mode == 8)
      {
        if (start_t == 0) {
          start_t = board_millis();  
        }
        int elapsed = board_millis() - start_t;
        int mult = 1 + (elapsed / 1000);  

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, -3*mult, 3*mult, 0, 0);
      }
    }
      
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
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
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, btn);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

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
      }else
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
