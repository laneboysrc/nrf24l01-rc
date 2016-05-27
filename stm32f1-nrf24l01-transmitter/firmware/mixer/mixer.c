#include <mixer.h>


int32_t channels[NUMBER_OF_CHANNELS];


// ****************************************************************************
void init_mixer(void)
{
    for (ch_t i = FIRST_HARDWARE_CHANNEL; i <= LAST_HARDWARE_CHANNEL; i++) {
        channels[i] = 0;
    }
}