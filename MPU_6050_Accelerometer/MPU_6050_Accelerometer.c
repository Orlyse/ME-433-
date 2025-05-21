#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include <stdlib.h>
#include <math.h>

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define ACC_M  0b1101000    // IMU address


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
    //printf("\nStarting write... \r\n");
    int written = i2c_write_blocking(I2C_PORT, ACC_M, buf, 2, false);
    //printf("Num bytes written = %d\r\n", written);
    sleep_ms(100);
}

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

void data_out(uint8_t* data_buf, int16_t *measurements){
    printf("\nCombining data bytes\r\n");
    int curr = 0;

    for (int i=0; i<7; i++){
        int16_t val1  = data_buf[curr];     // MSB
        int16_t val2 = data_buf[curr+1];    // LSB

        measurements[i] = (val1 << 8) | val2;
        curr += 2;
    }
    printf("\nData computation done...\r\n");
}

void read_all_data(int8_t *buf){
    
    uint8_t curr_reg = ACCEL_XOUT_H;

    for (int i=0; i<14; i++){
        i2c_read_byte(curr_reg, &buf[i]);
        curr_reg += 1;
    }

    printf("\nReading Completed...\n");
}

void display_IMU_readings(int16_t* measures){
    float ax = measures[0]/16384.0;
    float ay = measures[1]/16384.0;
    float az = measures[2]/16384.0;
    float temp = (measures[3]+521)/340.0;
    float rx = measures[4]/16.4;
    float ry = measures[5]/16.4;
    float rz = measures[6]/16.4;

    printf("Acc x = %.2f\r\n", ax);
    printf("Acc y = %.2f\r\n", ay);
    printf("Acc z = %.2f\r\n", az);

    printf("Temp = %.2f\r\n", temp);
    
    printf("Rot x = %.2f\r\n", rx);
    printf("Rot y = %.2f\r\n", ry);
    printf("Rot z = %.2f\r\n", rz);

}

void draw_dir(int magnitude, int x, int y){
    int len = abs(magnitude);

    for (int i=0; i<len; i++){
        draw_char(x+magnitude, y+i, '_');
    }
}

void draw_line_hor(int x0, int y0, int x1){
    int dx = (x1 > x0) ? 1 : -1;  // Step direction
    for (int x = x0; x != x1; x += dx){
        draw_char(x, y0, '_');
    }
    draw_char(x1, y0, '_'); // Include last point
}

void draw_line_ver(int x0, int y0, int y1){
    int dy = (y1 > y0) ? 1 : -1;  // Step direction
    for (int y = y0; y != y1; y += dy){
        draw_char(x0, y, '|');
    }
    draw_char(x0, y1, '|'); // Include last point
}

void acc_rep(int16_t* measures){
    float ax = (measures[0]/16384.0)*9.81;
    float ay = (measures[1]/16384.0)*9.81;
    
    int x0 = 63;
    int y0 = 15;

    int x1 = x0+(int) ax*10;
    int y1 = y0+(int) ay*10;

    if (x1!=x0){
        draw_line_hor(x0, y0, x1);
    }
    
    if (y1!=y0){
        draw_line_ver(x0, y0, y1);
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

    
    // Initialize IMU
    initialize_MPU6050();
    // Initialize display
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();
 
    printf("\rReading all data registers:\r\n");
    sleep_ms(100);

    /*  REGISTER DESCRIPTION: 
        ACCEL_XOUT_H 0x3B
        ACCEL_XOUT_L 0x3C
        Accelaration about the x axis in gs
        
        ACCEL_YOUT_H 0x3D
        ACCEL_YOUT_L 0x3E
        Accelaration about the x axis in gs


        ACCEL_ZOUT_H 0x3F
        ACCEL_ZOUT_L 0x40
        Accelaration about the x axis in gs

        TEMP_OUT_H   0x41
        TEMP_OUT_L   0x42
        Temperature in degrees celcius

        GYRO_XOUT_H  0x43
        GYRO_XOUT_L  0x44
        Acceleration about x in deg/sec

        GYRO_YOUT_H  0x45
        GYRO_YOUT_L  0x46
        Acceleration about y in deg/sec

        GYRO_ZOUT_H  0x47
        GYRO_ZOUT_L  0x48
        Acceleration about z in deg/sec
    */
    
    while (true) {
        ssd1306_clear();
        //draw_char(63, 16, '[');
        //draw_char(64, 16, '_');
        //draw_char(65, 16, ']');
        
        int8_t all_data[14];
        read_all_data(all_data);

        int16_t measures[7];
        data_out(all_data, measures);
        display_IMU_readings(measures);
        acc_rep(measures);

        ssd1306_update();
        
        sleep_ms(1000);
        
    }
    
}
