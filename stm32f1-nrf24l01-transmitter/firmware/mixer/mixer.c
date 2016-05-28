#include <stdint.h>

#include <inputs.h>
#include <mixer.h>


int32_t channels[NUMBER_OF_CHANNELS];


// ****************************************************************************
void MIXER_evaluate(void)
{
    INPUTS_adc_filter();

    // Start at ADC channel 1 as 0 is used for battery voltage
    for (unsigned i = 1; i < NUMBER_OF_ADC_CHANNELS; i++) {
        channels[i-1] = INPUTS_get_channel(i);
    }
}


// ****************************************************************************
void MIXER_init(void)
{
    for (ch_t i = FIRST_HARDWARE_CHANNEL; i <= LAST_HARDWARE_CHANNEL; i++) {
        channels[i] = 0;
    }
}