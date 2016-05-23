#include <systick.h>
#include <protocol_hk310.h>

#define FRAME_TIME 5        // One frame every 5 ms


typedef enum {
    SEND_STICK1 = 0,
    SEND_STICK2,
    SEND_BIND_INFO,
    SEND_PROGRAMBOX
} frame_state_t;

static frame_state_t frame_state;


// ****************************************************************************
static void send_stick_data(void)
{

}


// ****************************************************************************
static void send_binding_data(void)
{

}


// ****************************************************************************
static void send_programming_box_data(void)
{

}



// ****************************************************************************
static void nrf_transmit_done_callback(void)
{
    switch (frame_state) {
        case SEND_STICK1:
            send_stick_data();
            frame_state = SEND_BIND_INFO;
            break;

        case SEND_STICK2:
            send_stick_data();
            frame_state = SEND_BIND_INFO;
            break;

        case SEND_BIND_INFO:
            send_binding_data();
            frame_state = SEND_PROGRAMBOX;
            break;

        case SEND_PROGRAMBOX:
            send_programming_box_data();
            frame_state = SEND_STICK1;
            break;

        default:
            break;
    }
}


// ****************************************************************************
static void protocol_frame_callback(void)
{
    systick_set_callback(protocol_frame_callback, FRAME_TIME);
    frame_state = SEND_STICK1;
    nrf_transmit_done_callback();
}


// ****************************************************************************
void init_protocol_hk310(void)
{
    systick_set_callback(protocol_frame_callback, FRAME_TIME);
}


