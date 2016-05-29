#pragma once

#include <stdint.h>

#include <curves.h>
#include <inputs.h>


#define MAX_MIXER_UNITS 20

typedef struct  {
    curve_t curve;
    label_t src;
    label_t dest;
    uint8_t sw;
    int8_t scalar;
    int8_t offset;
    unsigned invert_source : 1;
} mixer_unit_t;


void MIXER_init(void);
void MIXER_evaluate(void);
