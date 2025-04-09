#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define GREEN_LED 16
#define LED_DELAY 500
#define BUTTON 17

static char event_str[128];
static volatile int count = 0;

// Included functions
void gpio_event_string(char *buf, uint32_t events);
void gpio_callback(uint gpio, uint32_t events);
int led_init(void);
void set_led(bool led_on_off);

//  Converts event to readable string

static const char *gpio_irq_str[] = {
    "LEVEL_LOW",  // 0x1
    "LEVEL_HIGH", // 0x2
    "EDGE_FALL",  // 0x4
    "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}

int led_init(void){
    gpio_init(GREEN_LED);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
}

void set_led(bool led_on_off){
    gpio_put(GREEN_LED, led_on_off);
}

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);
    count += 1;

    set_led(true);
    sleep_ms(LED_DELAY);
    set_led(false);
    sleep_ms(LED_DELAY);
    
    printf("Button count = %d\r\n", count);
}

int main()
{
    stdio_init_all();
    int rc = led_init();
    hard_assert(rc == PICO_OK);

    gpio_init(BUTTON);
    //  gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    while (true) {    
        ;  }
}
