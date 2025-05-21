#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"


// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define LEDPin 18

int draw_char(int x, int y, char letter){
    int ascii = letter - 32;
    //printf("ascii value = %d\r\n", ascii);

    for(int i=0; i<5; i++){ // i = x
        uint8_t hex_val = ASCII[ascii][i];
        //printf("ascii[%d][%d] = %x\r\n", ascii, i, hex_val);
        
        if (hex_val){
            int bit1 = (hex_val >> 7) & 0x01;
            int bit2 = (hex_val >> 6) & 0x01;
            int bit3 = (hex_val >> 5) & 0x01;
            int bit4 = (hex_val >> 4) & 0x01;
            int bit5 = (hex_val >> 3) & 0x01;
            int bit6 = (hex_val >> 2) & 0x01;
            int bit7 = (hex_val >> 1) & 0x01;
            int bit8 = hex_val & 0x01;

            uint8_t y_vals[8] = {bit8, bit7, bit6, bit5, bit4, bit3, bit2, bit1};
            
            for (int j=0; j<8; j++){
                //printf("bit %d = %d\r\n", j, y_vals[j]);
                if (y_vals[j]){
                    ssd1306_drawPixel(i+x, j+y, 1);
                }              

            }
        }
    }
}

void write_sentence (int x, int y, char* sent){
    int curr = 0;
    int step = 0;

    while(sent[curr] != '\0'){
        draw_char(x+step, y, sent[curr]);
        curr += 1;
        step += 6;
    }
}


int main()
{
    stdio_init_all();
    adc_init();
    gpio_init(LEDPin);
    gpio_set_dir(LEDPin, GPIO_OUT);
    adc_select_input(0);    //ADC0 = gp26

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c
    
    // Blink LED
    // set_pwm(LEDPin, 60000, 250, 250);
    // Set up ssd
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    char l1 = 'a';
    char message[100];
    sprintf(message, "Orlyse");

    while (true) {
        
        // Blink LED at 1 Hz
        gpio_put(LEDPin, 0);
        sleep_ms(500);
        gpio_put(LEDPin, 1);

        // Read ADC value 
        uint16_t voltage = adc_read();
        float volt = voltage/1000.0;
        
        // Print message on screen
        char message[100];
        sprintf(message, "Voltage = %.2f", volt);

        char fps[100];
        unsigned int t1 = to_us_since_boot(get_absolute_time());

        ssd1306_clear();
        write_sentence(10, 10, message);

        unsigned int t2 = to_us_since_boot(get_absolute_time());
        unsigned int diff = t2-t1;
        sprintf(fps, "Fps = %d", diff);

        write_sentence(10, 20, fps);

        ssd1306_update();
              
        

    }
}
