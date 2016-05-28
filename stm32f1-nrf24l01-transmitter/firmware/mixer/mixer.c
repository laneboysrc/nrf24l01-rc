#include <stdint.h>

#include <curves.h>
#include <inputs.h>
#include <mixer.h>


int32_t channels[NUMBER_OF_CHANNELS];

static curve_t test_curve = {
    CURVE_DEADBAND, 0,
    {-20, 50, -100}
};

// ****************************************************************************
void MIXER_evaluate(void)
{
    INPUTS_filter_and_normalize();

    // Start at ADC channel 1 as 0 is used for battery voltage
    for (unsigned i = 1; i < NUMBER_OF_ADC_CHANNELS; i++) {
        if (i == 1) {
            channels[i-1] = CURVE_evaluate(INPUTS_get_input(i), &test_curve);
        }
        else {
            channels[i-1] = INPUTS_get_input(i);
        }
    }
}


// ****************************************************************************
void MIXER_init(void)
{
    for (ch_t i = FIRST_HARDWARE_CHANNEL; i <= LAST_HARDWARE_CHANNEL; i++) {
        channels[i] = 0;
    }
}