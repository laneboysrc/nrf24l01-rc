#pragma once

#include <stdint.h>

#define NUMBER_OF_ADC_CHANNELS 10

typedef enum {
    ST = 1
} inputs_t;


void INPUTS_init(void);
int32_t INPUTS_get_input(inputs_t input);
void INPUTS_filter_and_normalize(void);

