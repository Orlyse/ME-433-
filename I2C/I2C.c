#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define ADDRESS 0b0100000

void i2c_write_byte(uint8_t *buf){
    sleep_ms(100);
    printf("\nStarting write... \r\n");
    int written = i2c_write_blocking(I2C_PORT, ADDRESS, buf, 2, false);
    printf("Num bytes written = %d\r\n", written);
    sleep_ms(100);
}

void i2c_read_byte(uint8_t reg, uint8_t *buf){
    printf("\nStarting read...\r\n");

    int written = i2c_write_blocking(I2C_PORT, ADDRESS, &reg, 1, true); 
    printf("Written = %d\r\n", written);
    
    uint32_t read = i2c_read_blocking(I2C_PORT, ADDRESS, buf, 1, false);  
    printf("Num bytes read = %d\r\n", read);

}

void turn_on_LED(){
    uint8_t buf1[2];

    buf1[0] = 0b00001010;   // address of the OLAT register
    buf1[1] = 0b10000000;   // set gp7 HIGH
    i2c_write_byte(buf1);
}

void turn_off_LED(){
    uint8_t buf2[2];

    buf2[0] = 0b00001010;   // address of the OLAT register
    buf2[1] = 0b00000000;   // set gp7 HIGH
    i2c_write_byte(buf2);
}
void read_gp0(uint8_t reg, uint8_t *gp0_read){
    i2c_read_byte(reg, gp0_read);
    uint8_t gp0_v = *gp0_read & 0x01;

    if (gp0_v == 1){
        turn_off_LED();
        printf("BUTTON NOT PRESSED\r\n");
    }
    else{
        turn_on_LED();
        printf("BUTTON PRESSED\r\n");
    }
}

int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    //while(!stdio_usb_connected()){
    //    sleep_ms(1000);
    //}

    uint8_t buf[2];
    buf[0] = 0;           // address of the IODIR register
    buf[1] = 0b00001111; // set gp7, gp6, gp5, gp4 as outputs and the rest as inputs

    i2c_write_byte(buf);

    printf("Starting I2C...\r\n");

    while (true){
        
        uint8_t gpio_reg = 0b00001001;     // address of gpio register
        uint8_t read;
        
        read_gp0(gpio_reg, &read);
        printf("Done\r\n"); 
    }

    return 0;   
}
