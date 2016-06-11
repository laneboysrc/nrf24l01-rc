#include <stdint.h>

#include <channels.h>
#include <config.h>
#include <limits.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


// ****************************************************************************
void LIMITS_apply(void)
{
    for (int i = 0; i < NUMBER_OF_OUTPUT_CHANNELS; i++) {
        limits_t *l = &config.model.limits[i];
        int32_t value = channels[FIRST_OUTPUT_CHANNEL_INDEX + i];

        // Map the channel CHANNEL_100_NPERCENT .. 0 .. CHANNEL_100_PERCENT to
        // (ep_l .. 0 .. ep_h) + subtrim.
        // This way the end points dictate throw (not hard stop) and the subtrim
        // dictates the center around the throws
        if (value >= 0) {
            output_channels[i] = l->subtrim + l->ep_h * value / CHANNEL_100_PERCENT;
        }
        else {
            output_channels[i] = l->subtrim + l->ep_l * value / CHANNEL_N100_PERCENT;
        }

        // Clamp to the configurable limits
        output_channels[i] = MAX(output_channels[i], l->limit_l);
        output_channels[i] = MIN(output_channels[i], l->limit_h);

        // Clamp to hard-coded limits of +/-180% to ensure valid servo pulses
        output_channels[i] = MAX(output_channels[i], HARD_LIMIT_L);
        output_channels[i] = MIN(output_channels[i], HARD_LIMIT_H);

        // Set the failsafe value, applying subtrim
        failsafe[i] = l->subtrim + l->failsafe;
    }
}
