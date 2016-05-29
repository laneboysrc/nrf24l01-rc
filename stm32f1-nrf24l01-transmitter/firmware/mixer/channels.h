#include <stdint.h>

// A "channel" has a value range from -10000..0..10000, corresponding to
// -100%..0..100%. This range is the same used in Deviation. It provides
// good resolution and it is human readable.
//
// This range applies to the normalized input channels as well as to the
// output channels.
//
// The normalized input channels are clamped to -10000..0..10000, while
// the output channels can go up to -18000..0..18000 (-180%..0%..180%),
// corresponding to receiver pulses of 600us..1500us..2400us
#define CHANNEL_100_PERCENT 10000
#define CHANNEL_CENTER 0
#define CHANNEL_N100_PERCENT -10000

#define CHANNEL_TO_PERCENT(x) ((x) / 100)
#define PERCENT_TO_CHANNEL(x) ((x) * 100)

#define NUMBER_OF_OUTPUT_CHANNELS 8
#define NUMBER_OF_VIRTUAL_CHANNELS 10
#define NUMBER_OF_CHANNELS (NUMBER_OF_OUTPUT_CHANNELS + NUMBER_OF_VIRTUAL_CHANNELS)

// Index of the channel that represents an output within channels[]
#define FIRST_OUTPUT_CHANNEL_INDEX 0

extern int32_t channels[NUMBER_OF_CHANNELS];
extern int32_t output_channels[NUMBER_OF_OUTPUT_CHANNELS];
