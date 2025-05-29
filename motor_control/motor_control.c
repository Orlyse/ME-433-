#include <stdio.h>
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

#define PWM 19 
#define PHASE 18

static volatile int current_pwm = 100;

void phase_pin_init(){
    gpio_init(PHASE);
    gpio_set_dir(PHASE, GPIO_OUT);

}
void set_pwm(int pin, uint16_t wrap, float divider, float level){
    // freq = 150M / Divider*wrap
    // Duty cyce = Level/wrap

    gpio_set_function(pin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(pin); // Get PWM slice number
    //float div = 3; // must be between 1-255
    pwm_set_clkdiv(slice_num, divider); // divider
    //uint16_t wrap = 50000; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
    pwm_set_gpio_level(pin, level); // 0 < level < wrap
}

void duty_cycle_set(char* dc){
    //printf("char dc = %c\n", *dc);

    if (dc[0] == '+'){
        if (current_pwm == 100){
            ; // do nothing
        }
        else{
            //printf("Addiing...\r\n");
            current_pwm += 10;
        }
    }

    else if (dc[0] == '-'){
        if (current_pwm == -100){
            ;   // do nothing
        }
    

        else{
            current_pwm -= 10;
            //printf("Subtracting...\r\n");
        }
    }
}

void dc_compute(int pwm_value){
    int abs_pwm = abs(current_pwm);

    if (pwm_value<0){
        gpio_put(PHASE, false);
    }
    else{
        gpio_put(PHASE, true);
    }

    set_pwm(PWM, 100, 150, abs_pwm);
}

int main()
{
    stdio_init_all();
    phase_pin_init();

    while(!stdio_usb_connected()){
        sleep_ms(10);
    }

    set_pwm(PWM, 100, 150, current_pwm);

    

    while (true) {
        char new_pwm[10];
        scanf("%s", new_pwm); 
        sleep_ms(100);

        duty_cycle_set(new_pwm);
        printf("Current pwm = %d\r\n", current_pwm);
        dc_compute(current_pwm);

        
    }
}
