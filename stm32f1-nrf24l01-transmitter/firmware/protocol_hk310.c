#include <systick.h>
#include <protocol_hk310.h>

#define FRAME_TIME 5        // 1 frame every 5 ms


// ****************************************************************************
static void protocol_frame_callback(void)
{
    systick_set_callback(protocol_frame_callback, FRAME_TIME);
}


// ****************************************************************************
void init_protocol_hk310(void)
{
    systick_set_callback(protocol_frame_callback, FRAME_TIME);
}



