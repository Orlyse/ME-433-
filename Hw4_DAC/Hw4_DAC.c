#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>
#include"Hw4_DAC.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// FUNCTIONS

static inline void cs_select(uint cs_pin);
static inline void cs_deselect(uint cs_pin);
float calculate_binary_value(float value);
void write_DAC(int channel, int voltage);
void sin_wave();
void triangle_wave();

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

float calculate_binary_value(float value){

    // input float is between -1 and 1, output binary value must be between 0 and 1023
    float normalized = (value + 1)/2; // between 0 and 1
    
    // normalize to go between 0 and 1023
    float D = normalized*1023;
    return D;
}

void write_DAC(int channel, int voltage){
    uint8_t data[2];
    int len = 2;
    
    data[0] = 0b01110000;   // 8-15: A/B, BUF, GA, SHDN, D9, D8, D7, D6
    data[0] = data[0] | (channel<<7); 

    uint8_t f1 = (voltage>>6)&0x0F;
    data[0] = data[0] | f1;

    uint8_t f2 = voltage << 2; //0b0000001110101101, 0b 0000 1110 (101101) 00 
    data[1] = f2 & 0xFFFC;   // 0-7: D5, D4, D3, D2, D1, D0, x, x
    
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
}

void sin_wave(){
    while (true) {
        
        float angle = 0;

        for(int i = 0; i<360; i++){

            float rad = sin((angle*M_PI)/180);
            uint16_t result = (uint16_t)calculate_binary_value(rad);
            write_DAC(0, result);
            angle += 1;
            sleep_ms(0.0001);
        }
    }
}

void triangle_wave(){
    /*
    Eqn y(t) = (2*A)/pi * arcsin(sin(2*pi/p)*x)
    Using p = 0.5
          A = 3.3
    */
   

    while (1){

        for (int i = 0; i<360; i++){

            float rad = i*M_PI/180; // Radian value of i which is in degrees
            float ang = (2/M_PI)*asin(sin(4*rad)); //    using p = 0.5, A = 1

            uint16_t result = (uint16_t)calculate_binary_value(ang);

            write_DAC(1, result);
            sleep_ms(10);
        }
    }
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*12.5);   // set baud rate to 1000 to easily see on the N-scope
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi


    sin_wave();
    //triangle_wave();
}
