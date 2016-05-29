#pragma once

#include <stdint.h>

#define NUMBER_OF_ADC_CHANNELS 10
#define MAX_LABELS 5
#define MAX_TRANSMITTER_INPUTS 20
#define MAX_LOGICAL_INPUTS 20

typedef enum {
    I_ST = 1
} inputs_t;

typedef uint8_t port_t;



// This structure describes the inputs on the transmitter PCB. It describes
// which ananlog inputs there are, and which digital inputs there are.
//
// Note that it does not describe whether those inputs are used or not, and to
// which functions they are connected. That is described in input_map.
typedef enum {
    ANALOG_DIGITAL,
    DIGITAL
} pcb_input_type_t;

typedef struct {
    pcb_input_type_t *type;
    port_t input;
} pcb_input_t;

extern const pcb_input_t pcb_inputs[];


// Here we define the low-level properties of each input as it is utilized in
// the transmitter hardware. An array of elements with this structure allows
// us to configure the inputs in correspondance to how they are wired up to the
// sticks, pots and switches in the transmitter.
//
// Note that it does not describe what the inputs *do*, only their low-level
// properties
typedef enum {
    TRANSMITTER_INPUT_NOT_USED = 0,
    ANALOG_WITH_CENTER,                 // CHANNEL_N100_PERCENT .. 0 .. CHANNEL_100_PERCENT
    ANALOG_NO_CENTER,                   // CHANNEL_N100_PERCENT .. CHANNEL_100_PERCENT
    ANALOG_NO_CENTER_POSITIVE_ONLY,     // 0 .. CHANNEL_100_PERCENT
    DIGITAL_ACTIVE_LOW,
    DIGITAL_ACTIVE_HIGH,
} transmitter_input_type_t;

typedef struct {
    port_t input;
    transmitter_input_type_t type;
    uint16_t calibration[3];             // Left/Center/Right HW endpoint calibration values
} transmitter_input_t;


// Finally we have the logical inputs, i.e. Steering, Throttle, Rudder, Elevator ...
// but also trims, dual-rate switches or pots, etc

typedef enum {
    ANALOG,
    SWITCH,
    PUSH_BUTTON,
    BCD_ENCODER
} input_type_t;

typedef enum {
    NONE = 0,
    ST,
    TH,
    THR,
    RUD,
    AIL,
    ELE,

    CH1,
    CH2,
    CH3,
    CH4,
    CH5,
    CH6,
    CH7,
    CH8
} label_t;

typedef struct {
    input_type_t type;
    uint8_t position_count;
    port_t inputs[12];
    label_t labels[MAX_LABELS];
} logical_input_t;



void INPUTS_init(void);
int32_t INPUTS_get_input(label_t input);
void INPUTS_filter_and_normalize(void);

