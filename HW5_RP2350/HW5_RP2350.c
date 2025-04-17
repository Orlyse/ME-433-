#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

//  FUNCTIONS
void computation_time();

void computation_time(){
    volatile float f1, f2;
    printf("Enter two floats to use:\n");
    scanf("%f %f", &f1, &f2);
    volatile float f_add, f_sub, f_mult, f_div = 0;

    // Addition
    absolute_time_t start1 = get_absolute_time();
    for (int i=0; i<1000; i++){
        f_add = f1+f2;
    }
    absolute_time_t t_add = get_absolute_time()-start1;
    uint64_t t_add_ms = to_us_since_boot(t_add);
    //  Subtraction    
    absolute_time_t start2 = get_absolute_time();
    for (int k = 0; k<1000; k++){
        f_sub = f1-f2;
    }
    absolute_time_t t_sub = get_absolute_time() - start2;
    uint64_t t_sub_ms = to_us_since_boot(t_sub);
    // Multiplication
    absolute_time_t start3 = get_absolute_time();
    for (int j = 0; j<1000; j++){
        f_mult = f1*f2;
    }
    absolute_time_t t_mult = get_absolute_time() - start3;
    uint64_t t_mult_ms = to_us_since_boot(t_mult);
    // Division
    absolute_time_t start4 = get_absolute_time();
    for (int l = 0; l<1000; l++){
        f_div = f1/f2;
    }
    absolute_time_t t_div = get_absolute_time() - start4;
    uint64_t t_div_ms = to_us_since_boot(t_div);

    printf("Time to add = %llums\n", t_add_ms);
    printf("Time to subtract = %llums\n",t_sub_ms);
    printf("Time to multiply = %llums\n", t_mult_ms);
    printf("Time to divide = %llums\n",t_div_ms);
    
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    while(!stdio_usb_connected()){
        sleep_ms(1);
    }
   
    computation_time();

    /*
    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
    */
}
