
#include <config.h>
#include <limits.h>
#include <mixer.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))



void LIMITS_apply(void)
{
    for (int i = 0; i < NUMBER_OF_HARDWARE_CHANNELS; i++) {
        limits_t *l = &config.model.limits[i];

        // Map the channel CHANNEL_100_NPERCENT .. 0 .. CHANNEL_100_PERCENT to
        // ep_l .. subtrim .. ep_h. This way the subtrim does not influence
        // the end points and vice versa.
        if (channels[i] >= 0) {
            channels[i] = l->subtrim + (l->ep_h - l->subtrim) * channels[i] / CHANNEL_100_PERCENT;
        }
        else {
            channels[i] = l->subtrim + (l->ep_l - l->subtrim) * channels[i] / CHANNEL_N100_PERCENT;
        }

        // Clamp to the configurable limits
        channels[i] = MAX(channels[i], l->limit_l);
        channels[i] = MIN(channels[i], l->limit_h);

        // Clamp to hard-coded limits of +/-180% to ensure valid servo pulses
        channels[i] = MAX(channels[i], HARD_LIMIT_L);
        channels[i] = MIN(channels[i], HARD_LIMIT_H);
    }
}
