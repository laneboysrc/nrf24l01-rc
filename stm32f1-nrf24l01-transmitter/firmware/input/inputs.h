#pragma once

#include <stdint.h>

#define NUMBER_OF_ADC_CHANNELS 10


void INPUTS_init(void);
int32_t INPUTS_get_channel(uint8_t ch);
void INPUTS_adc_filter(void);

