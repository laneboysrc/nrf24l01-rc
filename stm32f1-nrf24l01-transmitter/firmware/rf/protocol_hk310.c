#include <stdint.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include <libopencmsis/core_cm3.h>

#include <protocol_hk310.h>
#include <nrf24l01p.h>
#include <systick.h>
#include <mixer.h>

// NOTE: We keep the CE pin constantly high.
// While StandbyII mode (CE=1) consumes 320uA and StandbyI mode (CE=0) only
// 26uA, the difference is not important enough in our application to
// warrant having an additional IO pin in use, and an additional wire to
// drag around.


// ****************************************************************************
#define FRAME_TIME_MS 5        // One frame every 5 ms

#define ADDRESS_SIZE 5
#define PACKET_SIZE 10
#define NUMBER_OF_BIND_PACKETS 4
#define NUMBER_OF_HOP_CHANNELS 20
#define BIND_CHANNEL 81
#define FAILSAFE_PRESCALER_COUNT 17

typedef enum {
    SEND_STICK1 = 0,
    SEND_STICK2,
    SEND_BIND_INFO,
    SEND_PROGRAMBOX,
    FRAME_DONE
} frame_state_t;


static frame_state_t frame_state;
static uint8_t stick_packet[PACKET_SIZE];
static uint8_t failsafe_packet[PACKET_SIZE];
static uint8_t bind_packet[NUMBER_OF_BIND_PACKETS][PACKET_SIZE];
static uint8_t bind_packet_index = 0;
static uint8_t hop_index = 0;

// FIXME: those must come from a central registry
// TT01:
// c3da63c656
// 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67
static uint8_t hop_channels[NUMBER_OF_HOP_CHANNELS] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67};
static uint8_t address[ADDRESS_SIZE] = {0xc3, 0xda, 0x63, 0xc6, 0x56};

static const uint8_t bind_address[ADDRESS_SIZE] = {0x12, 0x23, 0x23, 0x45, 0x78};

// This counter determines how often failsafe packets are sent in relation to
// stick packets. The failsafe packets are sent every FAILSAFE_PRESCALER_COUNT.
// Ideally FAILSAFE_PRESCALER_COUNT should be a prime number so that the
// failsafe packet is sent on all hop channels over time.
static uint8_t failsafe_counter = 0;


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
// There are four bind packets:
//
// ff aa 55 a1 a2 a3 a4 a5 .. ..
// cc cc 00 ha hb hc hd he hf hg
// cc cc 01 hh hi hj hk hl hm hn
// cc cc 02 ho hp hq hr hs ht ..
static void build_bind_packets(void)
{
    uint16_t cc;

    // Put the constants in place: bind packet 0 identifier
    bind_packet[0][0] = 0xff;
    bind_packet[0][1] = 0xaa;
    bind_packet[0][2] = 0x55;

    // Put the constants in place: bind packet 1..3 index
    bind_packet[1][2] = 0x00;
    bind_packet[2][2] = 0x01;
    bind_packet[3][2] = 0x02;

    // Build the checksum for bind packets 1..3. It is simply the 16-bit
    // sum of the five address bytes.
    cc = 0;
    for (int i = 0; i < 5; i++) {
        cc += (uint16_t)address[i];
    }

    bind_packet[1][0] = cc & 0xff;
    bind_packet[2][0] = cc & 0xff;
    bind_packet[3][0] = cc & 0xff;

    bind_packet[1][1] = cc >> 8;
    bind_packet[2][1] = cc >> 8;
    bind_packet[3][1] = cc >> 8;

    // Put the address in bind packet 0
    for (int i = 0; i < 5; i++) {
        bind_packet[0][3+i] = address[i];
    }

    // Put the hop channels in bind packets 1..3
    for (int i = 0; i < 7; i++) {
        bind_packet[1][3+i] = hop_channels[i];
    }

    for (int i = 0; i < 7; i++) {
        bind_packet[2][3+i] = hop_channels[7+i];
    }

    for (int i = 0; i < 6; i++) {
        bind_packet[3][3+i] = hop_channels[14+i];
    }
}


// ****************************************************************************
static void send_stick_packet(void)
{
    nrf24_set_power(NRF24_POWER_0dBm);
    nrf24_write_register(NRF24_RF_CH, hop_channels[hop_index]);
    nrf24_write_multi_byte_register(NRF24_TX_ADDR, address, ADDRESS_SIZE);

    // Send failsafe packets instead of stick pacekts every
    // FAILSAFE_PRESCALER_COUNT times.
    if (failsafe_counter == 0) {
        nrf24_write_payload(failsafe_packet, PACKET_SIZE);
    }
    else {
        nrf24_write_payload(stick_packet, PACKET_SIZE);
    }

}


// ****************************************************************************
static void send_bind_packet(void)
{
    nrf24_write_register(NRF24_RF_CH, BIND_CHANNEL);
    nrf24_set_power(NRF24_POWER_n18dBm);
    nrf24_write_multi_byte_register(NRF24_TX_ADDR, bind_address, ADDRESS_SIZE);
    nrf24_write_payload(bind_packet[bind_packet_index], PACKET_SIZE);

    bind_packet_index = (bind_packet_index + 1) % NUMBER_OF_BIND_PACKETS;
}


// ****************************************************************************
static void send_programming_box_packet(void)
{
}


// ****************************************************************************
static void nrf_transmit_done_callback(void)
{
    // Clear the TX_DS status flags
    nrf24_write_register(NRF24_STATUS, NRF24_TX_DS);

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
            frame_state = FRAME_DONE;

            hop_index = (hop_index + 1) % NUMBER_OF_HOP_CHANNELS;
            failsafe_counter = (failsafe_counter + 1) % FAILSAFE_PRESCALER_COUNT;
            break;

        case FRAME_DONE:
        default:
            break;
    }
}


// ****************************************************************************
static void hk310_protocol_frame_callback(void)
{
    pulse_to_stickdata(channel_to_pulse(channels[CH1]), &stick_packet[0]);
    pulse_to_stickdata(channel_to_pulse(channels[CH2]), &stick_packet[2]);
    pulse_to_stickdata(channel_to_pulse(channels[CH3]), &stick_packet[4]);

    frame_state = SEND_STICK1;
    nrf_transmit_done_callback();
}


// ****************************************************************************
// Interrupt handler for EXTI8 (our nRF IRQ goes to PA8, hence EXTI8)
void exti9_5_isr(void)
{
    exti_reset_request(EXTI8);
    nrf_transmit_done_callback();
}


// ****************************************************************************
void init_protocol_hk310(void)
{
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


    // GPIO PA8 setup for falling-edge IRQ, with a pull-down
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO8);
    gpio_clear(GPIOA, GPIO8);

    exti_select_source(EXTI8, GPIOA);
    exti_set_trigger(EXTI8, EXTI_TRIGGER_FALLING);
    exti_enable_request(EXTI8);
    nvic_enable_irq(NVIC_EXTI9_5_IRQ);


    // nRF24 initialization
    nrf24_write_register(NRF24_SETUP_AW, NRF24_ADDRESS_WIDTH_5_BYTES);

    // Disable Auto Acknoledgement on all pipes
    nrf24_write_register(NRF24_EN_AA, 0x00);

    // Set bitrate to 250 kbps
    nrf24_set_bitrate(250);

    // TX mode, 2-byte CRC, power-up, Enable TX interrupt
    //
    // IMPORTANT: reverse logic: setting one of the "mask interrupt" pins
    // disables the IRQ output, while having the bit cleared enables IRQ output!
    //
    // See nRF24L01+ specification v1.0, section "Register map table", page 57
    nrf24_write_register(NRF24_CONFIG,
        NRF24_EN_CRC | NRF24_CRCO | NRF24_PWR_UP | NRF24_RX_RD | NRF24_MAX_RT);

    systick_set_rf_callback(hk310_protocol_frame_callback, FRAME_TIME_MS);
}


