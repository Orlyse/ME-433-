#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/gpio.h"


#define LEDPin 18
#define IS_RGBW false
#define NUM_PIXELS 4
#define TEST_LED 15

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 16
#endif

// Check the pin is compatible with the platform
#if WS2812_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

int led_init(void){
    gpio_init(TEST_LED);
    gpio_set_dir(TEST_LED, GPIO_OUT);
}

void blink(){
    gpio_put(TEST_LED, true);
    sleep_ms(500);
    gpio_put(TEST_LED, false);
}

// link three 8bit colors together
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wsColor; 

// adapted from https://forum.arduino.cc/index.php?topic=8498.0
// hue is a number from 0 to 360 that describes a color on the color wheel
// sat is the saturation level, from 0 to 1, where 1 is full color and 0 is gray
// brightness sets the maximum brightness, from 0 to 1
wsColor HSBtoRGB(float hue, float sat, float brightness) {
    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;

    if (sat == 0.0) {
        red = brightness;
        green = brightness;
        blue = brightness;
    } else {
        if (hue == 360.0) {
            hue = 0;
        }

        int slice = hue / 60.0;
        float hue_frac = (hue / 60.0) - slice;

        float aa = brightness * (1.0 - sat);
        float bb = brightness * (1.0 - sat * hue_frac);
        float cc = brightness * (1.0 - sat * (1.0 - hue_frac));

        switch (slice) {
            case 0:
                red = brightness;
                green = cc;
                blue = aa;
                break;
            case 1:
                red = bb;
                green = brightness;
                blue = aa;
                break;
            case 2:
                red = aa;
                green = brightness;
                blue = cc;
                break;
            case 3:
                red = aa;
                green = bb;
                blue = brightness;
                break;
            case 4:
                red = cc;
                green = aa;
                blue = brightness;
                break;
            case 5:
                red = brightness;
                green = aa;
                blue = bb;
                break;
            default:
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                break;
        }
    }

    unsigned char ired = red * 255.0;
    unsigned char igreen = green * 255.0;
    unsigned char iblue = blue * 255.0;

    wsColor c;
    c.r = ired;
    c.g = igreen;
    c.b = iblue;
    return c;
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (g) << 8) |
            ((uint32_t) (r) << 16) |
            (uint32_t) (b);
}

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

void light_all(PIO pio, uint sm, uint8_t r, uint8_t g, uint8_t b ){
    int i;
    for(i=0;i<NUM_PIXELS;i++){
        blink();
        put_pixel(pio, sm, urgb_u32(r, g, b));
    }
}

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
0 to 180 degrees ==> level 375 to 1875

1 degree = 1/180 * 1500 = 4.167 level

*/
void move_motor(float degree){

    float lvl = degree*1500/180.0;
    float n_lvl = 1875 - lvl;

    set_pwm(LEDPin, 15000, 200, n_lvl);
}

void move_0_to_180_5_seconds(){
    float step = 1.8;
    float next = step;
    // step = 18;
    int num_steps = 100;
    
    for (int i=0; i<num_steps; i++){
        move_motor(next);
        next += step;
        sleep_ms(50);
    }
    next -= step;

    for (int j=0; j<num_steps+1; j++){
        move_motor(next);
        next -= step;
        sleep_ms(50);
    }
}

int main()
{
    stdio_init_all();
    led_init();
    
    // Set up motor
    set_pwm(LEDPin, 15000, 200, 1875);
    sleep_ms(10);

    uint slice = pwm_gpio_to_slice_num(LEDPin); // Get PWM slice number
    pwm_set_enabled(slice, false);  

    // Set up LEDs

    // todo get free sm
    PIO pio;
    uint sm;
    uint offset;

    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    
    float start_hue = 0.0;
    move_motor(0);

    float step=1.8;
    float next = 1.8;
    bool forward = true; 
    int num_steps = 100;
        
    while (true) {

        if (next > 180){
            next = 178.2;
            forward = false;
        }
        else if (next < 0)
        {
            next = 0;
            forward = true;
        }

        // move_0_to_180_5_seconds();
        for (int i = 0; i < NUM_PIXELS; i++) {
            
            float curr_hue = fmod((start_hue + (90.0*i)), 360);
            wsColor color =  HSBtoRGB(curr_hue, 1.0, 0.5);
            put_pixel(pio, sm, urgb_u32(color.r, color.g, color.b));
        }

        if (forward){
            move_motor(next);
            next += step;
        }
        else{
            move_motor(next);
            next -= step;
        }
        

        sleep_ms(50);
    
        start_hue += 3.6;

        if (start_hue >= 360.0){
            start_hue -= 360;
        }
    }
    
}
