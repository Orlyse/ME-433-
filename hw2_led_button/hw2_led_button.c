#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define GREEN_LED 16
#define LED_DELAY 1000
#define BUTTON 

static volatile int count = 0;

int led_init(void){
    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
}

void set_led(bool led_on_off){
    gpio_put(GREEN_LED, led_on_off);
}

int main()
{
    stdio_init_all();
    int rc = led_init();
    hard_assert(rc == PICO_OK);

    while (true) {
        printf("Count = %d\r\n", count);
        set_led(true);
        sleep_ms(LED_DELAY);
        set_led(false);
        sleep_ms(LED_DELAY);
    }
}
