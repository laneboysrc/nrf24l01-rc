#pragma once

#include <stdint.h>

void init_inputs(void);
int32_t input_get_channel(uint8_t ch);
void adc_filter(void);

