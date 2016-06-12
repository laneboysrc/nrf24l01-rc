#pragma once

#include <channels.h>
#include <inputs.h>
#include <limits.h>
#include <mixer.h>
#include <protocol_hk310.h>


typedef struct {
    struct {
        transmitter_input_t transmitter_inputs[MAX_TRANSMITTER_INPUTS];
        logical_input_t logical_inputs[MAX_LOGICAL_INPUTS];
        uint32_t bind_timeout_ms;
        uint8_t led_pwm_percent;
    } tx;

    struct {
        char name[16];
        mixer_unit_t mixer_units[MAX_MIXER_UNITS];
        limits_t limits[NUMBER_OF_CHANNELS];
        protocol_hk310_t protocol_hk310;
    } model;
} config_t;


extern config_t config;

void CONFIG_init(void);
void CONFIG_save(void);
void CONFIG_perform_flash_write(void);

