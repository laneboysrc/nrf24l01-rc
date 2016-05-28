#include <stdint.h>

#include <inputs.h>
#include <mixer.h>


int32_t channels[NUMBER_OF_CHANNELS];


// ****************************************************************************
void mixer_evaluate(void)
{
    adc_filter();

    // Start at ADC channel 1 as 0 is used for battery voltage
    for (unsigned i = 1; i < NUMBER_OF_ADC_CHANNELS; i++) {
        channels[i-1] = input_get_channel(i);
    }
}


// ****************************************************************************
void init_mixer(void)
{
    for (ch_t i = FIRST_HARDWARE_CHANNEL; i <= LAST_HARDWARE_CHANNEL; i++) {
        channels[i] = 0;
    }
}