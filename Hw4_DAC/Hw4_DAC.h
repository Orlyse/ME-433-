#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

static inline void cs_select(uint cs_pin);
static inline void cs_deselect(uint cs_pin);
float calculate_binary_value(float value);
void write_DAC(int channel, int voltage);
void sin_wave();
void triangle_wave();

