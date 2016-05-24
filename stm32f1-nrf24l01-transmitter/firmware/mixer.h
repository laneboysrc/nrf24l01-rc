#pragma once

#include <stdint.h>

typedef enum {
    // Channels that are being sent to the receiver
    CH1, CH2, CH3, CH4, CH5, CH6, CH7, CH8,

    // Virtual channels for use in the mixer
    VIRT1, VIRT2, VIRT3, VIRT4, VIRT5, VIRT6, VIRT7, VIRT8, VIRT9, VIRT10,

    NUMBER_OF_CHANNELS,

    FIRST_HARDWARE_CHANNEL = CH1,
    LAST_HARDWARE_CHANNEL = CH8,
    NUMBER_OF_HARDWARE_CHANNELS = LAST_HARDWARE_CHANNEL - FIRST_HARDWARE_CHANNEL + 1,

    FIRST_VIRTUAL_CHANNEL = VIRT1,
    LAST_VIRTUAL_CHANNEL = VIRT10,
    NUMBER_OF_VIRTUAL_CHANNELS = LAST_VIRTUAL_CHANNEL - FIRST_VIRTUAL_CHANNEL + 1,
} ch_t;

extern int32_t channels[NUMBER_OF_CHANNELS];

void init_mixer(void);
