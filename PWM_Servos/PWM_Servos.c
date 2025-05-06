#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#define LEDPin 18 // the built in LED on the Pico

void set_pwm(int pin, uint16_t wrap, float divider, float level){
    // freq = 150M / Divider*wrap
    // Duty cyce = Level/wrap

    gpio_set_function(pin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(LEDPin); // Get PWM slice number
    //float div = 3; // must be between 1-255
    pwm_set_clkdiv(slice_num, divider); // divider
    //uint16_t wrap = 50000; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
    pwm_set_gpio_level(pin, level); // set the duty cycle to 50%
}

/*
0 to 360 degrees ==> level 375 to 1875

1 degree = 1/360 * 1500 = 4.167 level

*/
void move_motor(float degree){

    float lvl = degree*1500/180.0;
    float n_lvl = 1875 - lvl;

    set_pwm(LEDPin, 15000, 200, n_lvl);

    /*
    uint slice = pwm_gpio_to_slice_num(LEDPin); // Get PWM slice number

    for(int i=0; i<5; i++){
        set_pwm(LEDPin, 15000, 200, 40);
        sleep_ms(800);  
        pwm_set_enabled(slice, false);  
        sleep_ms(800);
    }
    */
}

void move_motor_ccw(int angle){
    uint slice = pwm_gpio_to_slice_num(LEDPin); // Get PWM slice number
    pwm_set_output_polarity(slice, true, true);
    
    for(int i=0; i<5; i++){
        set_pwm(LEDPin, 15000, 200, 40);
        sleep_ms(800);  
        pwm_set_enabled(slice, false);  
        sleep_ms(800);
    }
}

int main()
{
    stdio_init_all();
    
    // always start motor at 0 degrees (all the way left)    
    set_pwm(LEDPin, 15000, 200, 1875);
    sleep_ms(1000);
    uint slice = pwm_gpio_to_slice_num(LEDPin); // Get PWM slice number
    pwm_set_enabled(slice, false);  


    
    move_motor(180);

    while (true) {
        ;
    }
}
