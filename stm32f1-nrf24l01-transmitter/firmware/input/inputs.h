#pragma once

#include <stdint.h>

#define NUMBER_OF_ADC_CHANNELS 10


void init_inputs(void);
int32_t input_get_channel(uint8_t ch);
void adc_filter(void);

