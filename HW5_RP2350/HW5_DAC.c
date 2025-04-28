#include "HW5_DAC.h"

/*
void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}
*/

void write_DAC(int channel, int voltage){
    
    uint8_t data[2];
    int len = 2;
    
    data[0] = 0b01110000;   // 8-15: A/B, BUF, GA, SHDN, D9, D8, D7, D6
    data[0] = data[0] | (channel<<7); 

    uint8_t f1 = (voltage>>6)&0x0F;
    data[0] = data[0] | f1;

    uint8_t f2 = voltage << 2; //0b0000001110101101, 0b 0000 1110 (101101) 00 
    data[1] = f2 & 0xFFFC;   // 0-7: D5, D4, D3, D2, D1, D0, x, x
    
    cs_select(PIN_CS_D);
    spi_write_blocking(SPI_PORT_D, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS_D);
    
}