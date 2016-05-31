#pragma once

#include <stdint.h>

#define ADDRESS_SIZE 5
#define NUMBER_OF_HOP_CHANNELS 20

typedef struct {
    uint8_t hop_channels[NUMBER_OF_HOP_CHANNELS];
    uint8_t address[ADDRESS_SIZE];
} protocol_hk310_t;

void PROTOCOL_HK310_init(void);
void PROTOCOL_HK310_enable_binding(void);
void PROTOCOL_HK310_disable_binding(void);
