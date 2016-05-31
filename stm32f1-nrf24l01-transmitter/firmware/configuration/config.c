#include <config.h>

config_t config = {
    .tx = {
        .transmitter_inputs = {
            {.input = 1, .type = ANALOG_WITH_CENTER,        // Ailerons
             .calibration = {0, 2152, 4093}},

            {.input = 2, .type = ANALOG_WITH_CENTER,        // Elevator
             .calibration = {ADC_VALUE_MIN, ADC_VALUE_HALF, ADC_VALUE_MAX}},

            {.input = 3, .type = ANALOG_WITH_CENTER,        // Rudder
             .calibration = {ADC_VALUE_MIN, ADC_VALUE_HALF, ADC_VALUE_MAX}},

            {.input = 4, .type = ANALOG_NO_CENTER,          // Throttle
             .calibration = {ADC_VALUE_MIN, ADC_VALUE_HALF, ADC_VALUE_MAX}},
        },
        .logical_inputs = {
            {.type = ANALOG, .inputs = {1}, .labels = {AIL}},
            {.type = ANALOG, .inputs = {2}, .labels = {ELE}},
            {.type = ANALOG, .inputs = {3}, .labels = {RUD, ST}},
            {.type = ANALOG, .inputs = {4}, .labels = {THR, TH}}
        },
        .led_pwm_percent = 30
    },

    .model = {
        .mixer_units = {
            {
                .src = AIL,
                .dest = CH1,
                .curve = {
                    .type = CURVE_EXPO,
                    .points = {50, 50}
                },
                .scalar = 100,
                .offset = 1
            },
            {
                .src = ELE,
                .dest = CH2,
                .curve = {
                    .type = CURVE_NONE,
                },
                .scalar = 100
            },
            {
                .src = THR,
                .dest = CH3,
                .curve = {
                    .type = CURVE_NONE,
                },
                .scalar = 100
            },
            {
                .src = RUD,
                .dest = CH4,
                .curve = {
                    .type = CURVE_NONE,
                },
                .scalar = 100
            },
            {
                .src = 0
            }
        },

        .limits =  {
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
        },
        .protocol_hk310 = {
            .hop_channels = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67},
            .address = {0xc3, 0xda, 0x63, 0xc6, 0x56}
        }
    }
};
