# include <stdio.h>
# include "HW5_DAC.h"
// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi1
#define PIN_MISO 12
#define PIN_CS   13
#define PIN_SCK  14
#define PIN_MOSI 11
// Instructions
#define WRITE_INST 0b00000010
#define READ_INST 0b00000011
#define WRITE_STATUS 0b00000001


//  FUNCTIONS
void computation_time();
void write_memory(uint16_t address, uint32_t value);
void read_memory(uint16_t address, uint8_t* data_return0, uint8_t* data_return1, uint8_t* data_return2, uint8_t* data_return3);
uint32_t combine(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
float read_32_bit(uint16_t address);
void set_sequential();