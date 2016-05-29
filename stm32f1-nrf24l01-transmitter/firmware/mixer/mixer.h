#pragma once

#include <stdint.h>

#include <curves.h>
#include <inputs.h>


// A "channel" has a value range from -10000..0..10000, corresponding to
// -100%..0..100%. This range is the same used in Deviation. It provides
// good resolution and it is human readable.
//
// This range applies to the normalized input channels as well as to the
// output channels.
//
// The normalized input channels are clamped to -10000..0..10000, while
// the output channels can go up to -18000..0..18000 (-180%..0%..180%),
// corresponding to receiver pulses of 600us..1500us..2400us
#define CHANNEL_100_PERCENT 10000
#define CHANNEL_CENTER 0
#define CHANNEL_N100_PERCENT -10000

#define CHANNEL_TO_PERCENT(x) ((x) / 100)
#define PERCENT_TO_CHANNEL(x) ((x) * 100)

#define MAX_MIXER_UNITS 20
#define FIRST_HARDWARE_CHANNEL 0
#define LAST_HARDWARE_CHANNEL 7

// typedef enum {
//     // Channels that are being sent to the receiver
//     CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8,

//     // Virtual channels for use in the mixer
//     VIRT1, VIRT2, VIRT3, VIRT4, VIRT5, VIRT6, VIRT7, VIRT8, VIRT9, VIRT10,

//     NUMBER_OF_CHANNELS,

//     FIRST_HARDWARE_CHANNEL = CH1,
//     LAST_HARDWARE_CHANNEL = CH8,
//     NUMBER_OF_HARDWARE_CHANNELS = LAST_HARDWARE_CHANNEL - FIRST_HARDWARE_CHANNEL + 1,

//     FIRST_VIRTUAL_CHANNEL = VIRT1,
//     LAST_VIRTUAL_CHANNEL = VIRT10,
//     NUMBER_OF_VIRTUAL_CHANNELS = LAST_VIRTUAL_CHANNEL - FIRST_VIRTUAL_CHANNEL + 1,
// } ch_t;

#define NUMBER_OF_CHANNELS (8 + 10)

typedef struct  {
    curve_t curve;
    label_t src;
    label_t dest;
    uint8_t sw;
    int8_t scalar;
    int8_t offset;
    unsigned invert_source : 1;
} mixer_unit_t;


extern int32_t channels[NUMBER_OF_CHANNELS];


void MIXER_init(void);
void MIXER_evaluate(void);
