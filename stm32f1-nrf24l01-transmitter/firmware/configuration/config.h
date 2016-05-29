#pragma once

#include <inputs.h>
#include <limits.h>
#include <mixer.h>
#include <protocol_hk310.h>


typedef struct {
    struct {
        transmitter_input_t transmitter_inputs[MAX_TRANSMITTER_INPUTS];
        logical_input_t logical_inputs[MAX_LOGICAL_INPUTS];
    } tx;

    struct {
        mixer_unit_t mixer_units[MAX_MIXER_UNITS];
        limits_t limits[NUMBER_OF_CHANNELS];
        protocol_hk310_t protocol_hk310;
    } model;
} config_t;


extern config_t config;