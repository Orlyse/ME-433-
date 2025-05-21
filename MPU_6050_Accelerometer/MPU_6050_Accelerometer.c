#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define ACC_M  0b1101000 

// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C

// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75

void i2c_read_byte(uint8_t reg, uint8_t *buf){
    //printf("\nStarting read...\r\n");

    int written = i2c_write_blocking(I2C_PORT, ACC_M, &reg, 1, true); 
    //printf("Written = %d\r\n", written);
    
    uint32_t read = i2c_read_blocking(I2C_PORT, ACC_M, buf, 1, false);  
    //printf("Num bytes read = %d\r\n", read);
}


void i2c_write_byte(uint8_t *buf){
    sleep_ms(100);
    printf("\nStarting write... \r\n");
    int written = i2c_write_blocking(I2C_PORT, ACC_M, buf, 2, false);
    printf("Num bytes written = %d\r\n", written);
    sleep_ms(100);
}

void initialize_MPU6050(){
    //  write 0x00 to the PWR_MGMT_1 register to turn the chip on
    uint8_t power_buf[2];
    power_buf[0] = PWR_MGMT_1;
    power_buf[1] = 0b0;
    i2c_write_byte(power_buf);
    //sleep_ms(100);

    // write to the ACCEL_CONFIG register, set the sensitivity to plus minus 2g
    uint8_t acc_sensitivity[2];
    acc_sensitivity[0] = ACCEL_CONFIG;
    acc_sensitivity[1] = 0x00;
    i2c_write_byte(acc_sensitivity);
    //sleep_ms(100);

    // write to the GYRO_CONFIG register, set the sensitivity to plus minus 2000 dps
    uint8_t gyro[2];
    gyro[0] = GYRO_CONFIG;
    gyro[1] = 0b00011000;
    i2c_write_byte(gyro);
    //sleep_ms(100);
}

void data_out(uint8_t* data_buf, uint16_t *measurements){
    int curr = 0;

    for (int i=0; i<7; i++){
        uint16_t val1  = data_buf[curr];     // MSB
        uint16_t val2 = data_buf[curr+1];    // LSB

        measurements[i] = (val1 << 8) | val2;
        curr += 2;
    }
}

void read_all_data(uint8_t *buf){
    
    uint8_t curr_reg = ACCEL_XOUT_H;

    for (int i=0; i<14; i++){
        i2c_read_byte(curr_reg, &buf[i]);
        curr_reg += 1;
    }

    printf("\nReading Completed...\n");
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

    while(!stdio_usb_connected()){
        sleep_ms(100);
    }
    initialize_MPU6050();

    uint8_t data=0;
    uint8_t who_am_i = WHO_AM_I;
    printf("\nReading from %x\r\n", who_am_i);
    sleep_ms(100);

    i2c_read_byte(who_am_i, &data);
    printf("Value at who am i = %x\r\n", data);

    printf("\rReading all data registers:\r\n");
    sleep_ms(100);
    
    uint8_t all_data[14];
    read_all_data(all_data);

    uint16_t measures[7];
    data_out(all_data, measures);

    uint8_t curr = ACCEL_XOUT_H;
    sleep_ms(100);

    for (int j=0; j<14; j++){
        printf("%d\n", j);
        sleep_ms(10);
        printf("Register %x = %d\r\n", curr, all_data[j]);
        curr += 1;
    }

    sleep_ms(100);

    int val=0;

    for (int k=0; k<7; k++){
        printf("%d --> %d\r\n", k, measures[k]);
    }

    /*
    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
    */
}
