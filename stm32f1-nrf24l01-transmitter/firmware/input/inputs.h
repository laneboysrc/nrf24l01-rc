#pragma once

#include <stdint.h>

#define NUMBER_OF_ADC_CHANNELS 10
#define NUMBER_OF_LABELS 5

typedef enum {
    I_ST = 1
} inputs_t;

typedef enum {
    NONE = 0,
    ST,
    TH,
    THR,
    RUD,
    AIL,
    ELE,
} label_t;

typedef enum {
    ANALOG_DIGITAL,
    ANALOG,
    DIGITAL
} input_type_t;


typedef struct {
    input_type_t *type;
    uint8_t index;
} hw_input_t;

extern const hw_input_t hw_inputs[];

typedef struct {
    hw_input_t *hw;
    input_type_t type;
    label_t labels[NUMBER_OF_LABELS];
    uint8_t sub_element;
    int32_t value[3];
    unsigned active_high : 1;
} input_map_element_t;



void INPUTS_init(void);
int32_t INPUTS_get_input(inputs_t input);
void INPUTS_filter_and_normalize(void);

