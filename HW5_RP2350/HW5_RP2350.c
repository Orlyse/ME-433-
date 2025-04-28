#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>
#include <HW5_RP2350.h>
#include "HW5_DAC.h"

union float_32bit{
    float f;
    uint32_t u;
};

void set_sequential(){
    // lower cs
    cs_select(PIN_CS);

    // send 8 bit write instruction
    uint8_t write[1];
    write[0] = WRITE_STATUS;
    spi_write_blocking(SPI_PORT, write, 1);

    // set status register to sequential
    uint8_t status[2];
    status[0] = 0b01000001;
    spi_write_blocking(SPI_PORT, status, 1);

     // high cs
     cs_deselect(PIN_CS);
}

void write_memory(uint16_t address, uint32_t value){
    // lower cs
    cs_select(PIN_CS);
    
    // set to write
    uint8_t write[1];
    write[0] = WRITE_INST;
    spi_write_blocking(SPI_PORT, write, 1);

    // set up Data MSB first
    uint8_t data[4];
    uint16_t f0 = value & 0xFFFF; // last half 
    uint16_t f1 = value >> 16; // first half
    data[3] = f0 & 0x00FF;  
    data[2] = f0 >> 8;
    data[1] = f1 & 0x00FF; 
    data[0] = f1 >> 8;   // MSB

    // Set up address
    uint8_t add[2];
    add[1] = address & 0x00FF;  // LSB
    add[0] = address >> 8 ;     // MSB 

    // send address
    spi_write_blocking(SPI_PORT, add, 2);
    // Send data
    spi_write_blocking(SPI_PORT, data, 4);

    // high cs
    cs_deselect(PIN_CS);
}

void read_memory(uint16_t address, uint8_t* data_return0, uint8_t* data_return1, uint8_t* data_return2, uint8_t* data_return3){
    // lower cs
    cs_select(PIN_CS);

    // send read instruction
    uint8_t read = READ_INST;
    spi_write_blocking(SPI_PORT, &read, 1);

    // set up address
    uint8_t add[2];
    add[1] = address & 0x00FF;  // LSB
    add[0] = address >> 8 ;     // MSB 
    
    // send address
    spi_write_blocking(SPI_PORT, add, 2);
    
    // read output
    uint8_t dummy = 0; 
    spi_read_blocking(SPI_PORT, dummy, data_return0, 1);
    spi_read_blocking(SPI_PORT, dummy, data_return1, 1);
    spi_read_blocking(SPI_PORT, dummy, data_return2, 1);
    spi_read_blocking(SPI_PORT, dummy, data_return3, 1);

    // high cs
    cs_deselect(PIN_CS);
}

uint32_t combine(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3){
    //  d0 = MSB
    uint32_t tot= ((uint32_t)d0 << 24) | ((uint32_t)d1 << 16) | ((uint32_t)d2 << 8) | (uint32_t)d3;
    return tot; 
}

float read_32_bit(uint16_t address){
    uint8_t data0[1];
    uint8_t data1[1];
    uint8_t data2[1];
    uint8_t data3[1];

    read_memory(address, data0, data1, data2, data3);

    uint32_t data_out = combine(*data0, *data1, *data2, *data3);

    union float_32bit f_value;
    f_value.u = data_out;
    
    return f_value.f;
}

float calculate_binary_value(float value){
    // input float is between -1 and 1, output binary value must be between 0 and 1023
    float normalized = (value + 1)/2; // between 0 and 1
    // normalize to go between 0 and 1023
    float D = normalized*1023;
    return D;
}

void load_sin_wave(){
    uint16_t address = 0;

    for (int i=0; i<1000; i++){
        // Calculate sin value 
        float rad = sin((i*M_PI)/180);
        //  printf("rad = %.2f\n", rad);
        float dac_val = calculate_binary_value(rad);
        //  printf("dac = %.2f\n", dac_val);
        union float_32bit value;
        value.f = dac_val;
        //  printf("value.f = %.2f\n", value.f);
        
        // Store in memory
        write_memory(address, value.u);
        
        address += 4;
    }
}

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
    long long cycles = (long long)( (t_add_ms / 1000.0) / 0.006667 );
    printf("Cycles to add once = %lld\n", cycles);

    printf("Time to subtract = %llums\n",t_sub_ms);
    long long cycles1 = (long long)( (t_sub_ms / 1000.0) / 0.006667 );
    printf("Cycles to add once = %lld\n", cycles1);

    printf("Time to multiply = %llums\n", t_mult_ms);
    long long cycles2 = (long long)( (t_mult_ms / 1000.0) / 0.006667 );
    printf("Cycles to multiply once = %lld\n", cycles2);

    printf("Time to divide = %llums\n",t_div_ms);
    long long cycles3 = (long long)( (t_div_ms / 1000.0) / 0.006667 );
    printf("Cycles to divide once = %lld\n", cycles3);
    
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

    // SPI initialisation for the DAC
    spi_init(SPI_PORT_D, 1000*12.5);   // set baud rate to 1000 to easily see on the N-scope
    gpio_set_function(PIN_MISO_D, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_D,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK_D,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI_D, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS_D, GPIO_OUT);
    gpio_put(PIN_CS_D, 1);

    // Initialization functions
    set_sequential();
    load_sin_wave();

    while(!stdio_usb_connected()){
        sleep_ms(1000);
    }

    uint16_t address = 0;

    printf("Starting loop...\n");
    computation_time();

    while(true){
        
        if(address > 4000){
            address = 0; // when you hit the last address, start again
        }

        float voltage = read_32_bit(address);
        // printf("V = %.2f\n", voltage);
        // printf("Address = %d\n", address);
        
        write_DAC(0, voltage); 
        sleep_ms(1);   
        address += 4;    
    }
            
}
