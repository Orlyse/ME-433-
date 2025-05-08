#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardware/gpio.h"

#define BUTTON_LEFT 10       // connected to a pull up resistor
#define BUTTON_DOWN 7       // connected to a pull up resistor
#define BUTTON_UP 8         // connected to a pull up resistor
#define BUTTON_RIGHT 19     // connected to a pull up resistor

void button_initialize(){
    gpio_init(BUTTON_UP);
    gpio_init(BUTTON_RIGHT);
    gpio_init(BUTTON_DOWN);
    gpio_init(BUTTON_LEFT);
  
    gpio_set_dir(BUTTON_UP, false);
    gpio_set_dir(BUTTON_RIGHT, false);
    gpio_set_dir(BUTTON_DOWN, false);
    gpio_set_dir(BUTTON_LEFT, false);
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

int main()
{
    stdio_init_all();
    button_initialize();

    
    while(!stdio_usb_connected())
    {
        sleep_ms(100);//  wait;
    }

    while (true) {
        int mode = mouse_mode();
        printf("Mode = %d\r\n", mode);
        sleep_ms(10);
    }
}
