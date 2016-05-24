#include <systick.h>
#include <protocol_hk310.h>
#include <mixer.h>

// ****************************************************************************
#define FRAME_TIME 5        // One frame every 5 ms


typedef enum {
    SEND_STICK1 = 0,
    SEND_STICK2,
    SEND_BIND_INFO,
    SEND_PROGRAMBOX
} frame_state_t;


static frame_state_t frame_state;
static uint8_t stick_packet[10];
static uint8_t failsafe_packet[10];
static uint8_t bind_packet[4][10];

// FIXME: those must come from a central registry
// TT01:
// c3da63c656
// 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67
static uint8_t hop_channels[20] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67};
static uint8_t address[5] = {0xc3, 0xda, 0x63, 0xc6, 0x56};


// ****************************************************************************
static void pulse_to_stickdata(unsigned int pulse_ns, uint8_t *packet_ptr)
{
    uint16_t stickdata;

    // Ensure that we are not overflowing the resulting uint16_t
    if (pulse_ns > 49151 * 1000) {
        pulse_ns = 49151 * 1000;
    }

    // Magic formula how the receiver converts stickdata into the generated
    // servo pulse duration.
    stickdata = 0xffff - (pulse_ns * 4 / (3 * 1000));

    packet_ptr[0] = stickdata & 0xff;
    packet_ptr[1] = stickdata >> 8;
}


// ****************************************************************************
static unsigned int channel_to_pulse(int32_t ch)
{
    int32_t pulse_ns;

    pulse_ns = (1500 * 1000) + (ch * 500 * 1000 / 0x7fff);

    if (pulse_ns < 0) {
        return 0;
    }

    return (unsigned int)pulse_ns;
}


// ****************************************************************************
static void build_bind_packets(void)
{
    // ff aa 55 a1 a2 a3 a4 a5 .. ..
    // cc cc 00 ha hb hc hd he hf hg
    // cc cc 01 hh hi hj hk hl hm hn
    // cc cc 02 ho hp hq hr hs ht ..

    uint16_t cc;

    cc = 0;
    for (int i = 0; i < 5; i++) {
        cc += (uint16_t)address[i];
    }

    bind_packet[0][0] = 0xff;
    bind_packet[0][1] = 0xaa;
    bind_packet[0][2] = 0x55;

    bind_packet[1][2] = 0x00;
    bind_packet[2][2] = 0x01;
    bind_packet[3][2] = 0x02;

    bind_packet[1][0] = cc & 0xff;
    bind_packet[2][0] = cc & 0xff;
    bind_packet[3][0] = cc & 0xff;

    bind_packet[1][1] = cc >> 8;
    bind_packet[2][1] = cc >> 8;
    bind_packet[3][1] = cc >> 8;

    for (int i = 0; i < 5; i++) {
        bind_packet[0][3+i] = address[i];
    }

    for (int i = 0; i < 7; i++) {
        bind_packet[0][3+i] = hop_channels[i];
    }

    for (int i = 0; i < 7; i++) {
        bind_packet[0][3+i] = hop_channels[7+i];
    }

    for (int i = 0; i < 6; i++) {
        bind_packet[0][3+i] = hop_channels[14+i];
    }
}


// ****************************************************************************
static void send_stick_packet(void)
{
    pulse_to_stickdata(channel_to_pulse(channels[CH1]), &stick_packet[0]);
    pulse_to_stickdata(channel_to_pulse(channels[CH2]), &stick_packet[2]);
    pulse_to_stickdata(channel_to_pulse(channels[CH3]), &stick_packet[4]);
}


// ****************************************************************************
static void send_bind_packet(void)
{

}


// ****************************************************************************
static void send_programming_box_packet(void)
{

}


// ****************************************************************************
static void nrf_transmit_done_callback(void)
{
    switch (frame_state) {
        case SEND_STICK1:
            send_stick_packet();
            frame_state = SEND_BIND_INFO;
            break;

        case SEND_STICK2:
            send_stick_packet();
            frame_state = SEND_BIND_INFO;
            break;

        case SEND_BIND_INFO:
            send_bind_packet();
            frame_state = SEND_PROGRAMBOX;
            break;

        case SEND_PROGRAMBOX:
            send_programming_box_packet();
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

    stick_packet[7] = 0x55;         // Packet ID for stick data

    // Failsafe for steering: 1200 us
    pulse_to_stickdata(1200*1000, &failsafe_packet[0]);

    // Failsafe for throttle: 1500 us (center!)
    pulse_to_stickdata(1500*1000, &failsafe_packet[2]);

    // Failsafe for CH3: 2000 us
    pulse_to_stickdata(2000*1000, &failsafe_packet[4]);

    failsafe_packet[7] = 0xaa;      // Packet ID for failsafe data
    failsafe_packet[8] = 0x5a;      // Failsafe on

    build_bind_packets();
}


