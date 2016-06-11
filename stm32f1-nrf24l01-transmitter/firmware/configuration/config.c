#include <config.h>

config_t config = {
    .tx = {
        .transmitter_inputs = {
            {.input = 1, .type = ANALOG_WITH_CENTER,        // Ailerons
             .calibration = {510, 1962, 3380}},

            {.input = 2, .type = ANALOG_WITH_CENTER,        // Elevator
             .calibration = {590, 1943, 3240}},

            {.input = 3, .type = ANALOG_WITH_CENTER,        // Throttle
             .calibration = {670, ADC_VALUE_HALF, 3370}},

            {.input = 4, .type = ANALOG_NO_CENTER,          // Rudder
             .calibration = {580, 1874, 3410}},
        },
        .logical_inputs = {
            {.type = ANALOG, .inputs = {1}, .labels = {AIL}},
            {.type = ANALOG, .inputs = {2}, .labels = {ELE}},
            {.type = ANALOG, .inputs = {3}, .labels = {THR, TH}},
            {.type = ANALOG, .inputs = {4}, .labels = {RUD, ST}}
        },
        .led_pwm_percent = 30,
        .bind_timeout_ms = 10 * 1000
    },

    .model = {
        .mixer_units = {
            {
                .src = AIL,
                .dest = CH1,
                .curve = {
                    .type = CURVE_NONE,
                    .points = {50, 50}
                },
                .scalar = 100,
                .offset = 0
            },
            {
                .src = ELE,
                .dest = CH2,
                .curve = {
                    .type = CURVE_NONE,
                },
                .scalar = 100,
                .offset = 0,
                .invert_source = 1
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
                .ep_l = PERCENT_TO_CHANNEL(-35), .ep_h = PERCENT_TO_CHANNEL(30), .subtrim = -1600,
                .limit_l = -150000, .limit_h = 150000,
                .failsafe = PERCENT_TO_CHANNEL(8)
            },
            {
                .ep_l = PERCENT_TO_CHANNEL(-35), .ep_h = PERCENT_TO_CHANNEL(30), .subtrim = 2200,
                .limit_l = -150000, .limit_h = 150000,
                .failsafe = PERCENT_TO_CHANNEL(5)
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
