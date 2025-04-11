#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define BUTTON 17
#define LED 16
#define VARIABLE_RESISTOR 26

void set_up(){
    adc_init();
    gpio_init(BUTTON);
    gpio_init(LED);
    adc_gpio_init(VARIABLE_RESISTOR); // GP26 = ADC0

    // set direction
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_set_dir(LED, GPIO_OUT);
    adc_select_input(0);    // ADC0
}

void toggle_led(){
    /*
    printf("button = %d\r\n", gpio_get(BUTTON));
    printf("led = %d\r\n", gpio_get(LED));
    sleep_ms(1000);
    */
    
    gpio_put(LED, 1);
    
    while (true){

        if (gpio_get(BUTTON)){
            gpio_put(LED, 0);
            sleep_ms(500);
            break;
        }
        
    }
    
    char samples[100];
    printf("Type sample size (1-100): ");
    scanf("%d", samples);
    int samps = *samples;
    //  printf("\nRequire %d samples\r\n", samps);
    
    uint16_t volt = adc_read();
    float voltage = volt/1000.0;

    for (int i=0; i< samps; i++){
        printf("V = %.2f\r\n", voltage);
        sleep_ms(10);
    }    
    
}

int main()
{
    stdio_init_all();
    set_up();
    
    while (!stdio_usb_connected()){
        sleep_ms(100);//  wait;
    }

    printf("Starting program....\r\n");

    while (true) {
        toggle_led();

        /*
        char message[100];
        scanf("%s", message);
        printf("message: %s\r\n", message);
        sleep_ms(100);
        */
    }
}
