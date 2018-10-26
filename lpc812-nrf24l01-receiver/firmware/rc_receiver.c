#include <stdint.h>
#include <stdbool.h>

#include <platform.h>
#include <rc_receiver.h>
#include <persistent_storage.h>
#include <rf.h>
#include <uart0.h>


#define STICKDATA_PACKETID_3CH 0x55
#define FAILSAFE_PACKETID_3CH 0xaa

#define STICKDATA_PACKETID_4CH 0x56
#define FAILSAFE_PACKETID_4CH 0xab

#define STICKDATA_PACKETID_8CH 0x57
#define FAILSAFE_PACKETID_8CH 0xac


#define PAYLOAD_SIZE 10
#define ADDRESS_WIDTH 5
#define NUMBER_OF_HOP_CHANNELS 20
#define MAX_HOP_WITHOUT_PACKET 15
#define FIRST_HOP_TIME_IN_US 2500
#define HOP_TIME_IN_US 5000

#define FAILSAFE_TIMEOUT (640 / __SYSTICK_IN_MS)
#define BIND_TIMEOUT (5000 / __SYSTICK_IN_MS)
#define BIND_SWAP_TIMEOUT (50 / __SYSTICK_IN_MS)
#define ISP_TIMEOUT (3000 / __SYSTICK_IN_MS)
#define BLINK_TIME_FAILSAFE (320 / __SYSTICK_IN_MS)
#define BLINK_TIME_BINDING (50 / __SYSTICK_IN_MS)



#define BUTTON_PRESSED 0
#define BUTTON_RELEASED 1

typedef enum {
    LED_STATE_UNKNOWN,
    LED_STATE_IDLE,
    LED_STATE_RECEIVING,
    LED_STATE_FAILSAFE,
    LED_STATE_BINDING,
} led_state_t;


extern bool systick;
extern void switch_gpio_according_rx_protocol(rx_protocol_t rx_protocol);


uint16_t channels[NUMBER_OF_CHANNELS];
uint16_t raw_data[2];
bool successful_stick_data = false;

static bool rf_int_fired = false;

static led_state_t led_state;

static unsigned int blink_timer;
static unsigned int bind_button_timer;

static uint8_t payload[RF_MAX_BUFFER_LENGTH];

static uint8_t failsafe_enabled;
static uint16_t failsafe[NUMBER_OF_CHANNELS];
static unsigned int failsafe_timer;

static uint8_t model_address[ADDRESS_WIDTH];
static bool perform_hop_requested = false;
static unsigned int hops_without_packet;
static unsigned int hop_index;
static uint8_t hop_data[NUMBER_OF_HOP_CHANNELS];

static bool binding_requested = false;
static bool binding = false;
static unsigned int bind_timer;
static unsigned int bind_swap_timer;
static const uint8_t BIND_CHANNEL = 0x51;
static const uint8_t BIND_ADDRESS[ADDRESS_WIDTH] = {0x12, 0x23, 0x23, 0x45, 0x78};
static uint8_t bind_storage_area[NUMBER_OF_PERSISTENT_ELEMENTS] __attribute__ ((aligned (4)));
#define PROTOCOLID_INDEX (sizeof(bind_storage_area)-1)

static uint8_t stickdata_packetid;
static uint8_t failsafe_packetid;

static rx_protocol_t rx_protocol;


// ****************************************************************************
// static void print_payload(void)
// {
//     int i;
//     for (i = 0; i < PAYLOAD_SIZE; i++) {
//         uart0_send_uint8_hex(payload[i]);
//         uart0_send_char(' ');
//     }
//     uart0_send_linefeed();
// }


// ****************************************************************************
static void initialize_failsafe(void) {
    int i;

    failsafe_enabled = false;
    failsafe_timer = FAILSAFE_TIMEOUT;
    for (i = 0; i < NUMBER_OF_CHANNELS; i++) {
        failsafe[i] = SERVO_PULSE_CENTER;
    }
}


// ****************************************************************************
static void output_pulses(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        LPC_SCT->MATCHREL[i + 1].H = channels[i];
    }
}


// ****************************************************************************
static uint16_t stickdata2timer(uint16_t stickdata)
{
    uint32_t ms;

    // ms = (0xffff - stickdata) * 3 / 4;
    ms = (0xffff - stickdata);
    return ms & 0xffff;
}


// ****************************************************************************
static uint16_t stickdata2timer8ch(uint16_t stickdata)
{
    return (476 * 2) + stickdata;
}


// ****************************************************************************
// This code undos the value scaling that the transmitter nRF module does
// when receiving a 12 bit channel value via the UART, while forming the
// transmit packet.
//
// The transmitter calculates
//
//    value = (uart_data * 14 / 10) + 0xf200
//
// As such the input range via the UART can be 0x000 .. 0x9ff
// ****************************************************************************
static uint16_t stickdata2txdata(uint16_t stickdata)
{
    uint32_t txdata;

    txdata = (stickdata - 0xf200) * 10 / 14;
    return txdata & 0xffff;
}


// ****************************************************************************
static void stop_hop_timer(void)
{
    // Stop the SCTimer L
    LPC_SCT->CTRL_L |= (1 << 2);

    perform_hop_requested = false;
}


// ****************************************************************************
static void restart_hop_timer(void)
{
    LPC_SCT->CTRL_L |= (1 << 2);
    LPC_SCT->MATCHREL[0].L = HOP_TIME_IN_US - 1;

    // We need to set the MATCH register, not the MATCHREL register here as
    // only after the first match the MATCHREL gets copied in!
    LPC_SCT->MATCH[0].L = FIRST_HOP_TIME_IN_US;

    LPC_SCT->COUNT_L = 0;
    LPC_SCT->CTRL_L &= ~(1 << 2);

    hops_without_packet = 0;
    perform_hop_requested = false;
}


// ****************************************************************************
static void restart_packet_receiving(void)
{
    stop_hop_timer();

    rf_clear_ce();
    hop_index = 0;
    hops_without_packet = 0;
    perform_hop_requested = false;

    rf_set_crc(CRC_2_BYTES);
    rf_set_irq_source(RX_RD);
    rf_set_address_width(ADDRESS_WIDTH);
    rf_set_data_rate(DATA_RATE_250K);
    rf_set_data_pipes(DATA_PIPE_0, NO_AUTO_ACKNOWLEDGE);

    // FIXME: set timer to 500ns for 8ch, 750ns for 3/4ch protocol

    if (rx_protocol == PROTOCOL_8CH) {
        // Enable dynamic payload length
        rf_set_feature(EN_DPL);
        // Enable dynamic payload length on pipe 0
        rf_set_dynpd(DATA_PIPE_0);
    }
    else {
        rf_set_payload_size(DATA_PIPE_0, PAYLOAD_SIZE);
        // Disable dynamic payout size on all pipes
        rf_set_dynpd(0);
        // Disable all dynamic features
        rf_set_feature(0);
    }

    rf_set_rx_address(DATA_PIPE_0, ADDRESS_WIDTH, model_address);
    rf_set_channel(hop_data[0]);

    rf_flush_rx_fifo();
    rf_clear_irq(RX_RD);
    rf_int_fired = false;
    rf_set_ce();
}


// ****************************************************************************
static void parse_bind_data(void)
{
    int i;

    for (i = 0; i < ADDRESS_WIDTH; i++) {
        model_address[i] = bind_storage_area[i];
    }

    for (i = 0; i < NUMBER_OF_HOP_CHANNELS; i++) {
        hop_data[i] = bind_storage_area[ADDRESS_WIDTH + i];
    }


    rx_protocol = bind_storage_area[PROTOCOLID_INDEX];

    switch (rx_protocol) {
        default:
            rx_protocol = PROTOCOL_3CH;
            // fall through
        case PROTOCOL_3CH:
            stickdata_packetid = STICKDATA_PACKETID_3CH;
            failsafe_packetid = FAILSAFE_PACKETID_3CH;
            break;

        case PROTOCOL_4CH:
            stickdata_packetid = STICKDATA_PACKETID_4CH;
            failsafe_packetid = FAILSAFE_PACKETID_4CH;
            break;

        case PROTOCOL_8CH:
            stickdata_packetid = STICKDATA_PACKETID_8CH;
            failsafe_packetid = FAILSAFE_PACKETID_8CH;
            break;
    }

    switch_gpio_according_rx_protocol(rx_protocol);
}


// ****************************************************************************
static void binding_done(void)
{
    led_state = LED_STATE_IDLE;
    failsafe_timer = FAILSAFE_TIMEOUT;
    binding = false;
    binding_requested = false;

    restart_packet_receiving();
}


// ****************************************************************************
// The bind process works as follows:
//
// The transmitter regularly sends data on the fixed channel 51h, with address
// 12:23:23:45:78.
// This data is sent at a lower power. All transmitters do that all the time.
//
// A bind data packet is sent every 5ms.
//
// The transmitter cycles through 4 packets:
// ff aa 55 a1 a2 a3 a4 a5 .. ..
// cc cc 00 ha hb hc hd he hf hg
// cc cc 01 hh hi hj hk hl hm hn
// cc cc 02 ho hp hq hr hs ht ..
//
// ff aa 55     Special marker for the first packet
// a[1-5]       The 5 address bytes
// cc cc        A 16 byte checksum of bytes a1..a5
// h[a-t]       20 channels for frequency hopping
// ..           Not used
//
// ****************************************************************************
static void process_binding(void)
{
    static enum {
        BIND_STATE_4CH_1,
        BIND_STATE_4CH_2,
        BIND_STATE_4CH_3,
        BIND_STATE_4CH_4,
        BIND_STATE_8CH,
    } bind_state;
    static uint16_t checksum;
    uint8_t payload_width = PAYLOAD_SIZE;
    int i;

    // ================================
    if (!binding) {
        if (!binding_requested) {
            return;
        }

        binding_requested = false;
        led_state = LED_STATE_BINDING;
        binding = true;
        bind_state = BIND_STATE_4CH_1;
        bind_timer = BIND_TIMEOUT;
        bind_swap_timer = 0;

#ifndef NO_DEBUG
        uart0_send_cstring("Starting bind procedure\n");
#endif
        return;
    }


    // ================================
    if (bind_timer == 0) {
#ifndef NO_DEBUG
        uart0_send_cstring("Bind timeout\n");
#endif
        binding_done();
        return;
    }


    // ================================
    // Every 50ms we toggle between the 3/4ch bind mode and 8ch bind mode.
    // We only do that if we haven't received one of the 3/4ch bind packets
    // yet (multiple bind packets are combined to form bind data)
    if (bind_swap_timer == 0) {
        bind_swap_timer = BIND_SWAP_TIMEOUT;

        if (bind_state == BIND_STATE_4CH_1) {
            bind_state = BIND_STATE_8CH;
            // Set special address 12h 23h 23h 45h 78h
            rf_set_rx_address(0, ADDRESS_WIDTH, BIND_ADDRESS);
            // Set special channel 0x51
            rf_set_channel(BIND_CHANNEL);

            rf_set_data_rate(DATA_RATE_2M);
            // Enable dynamic payload length
            rf_set_feature(EN_DPL);
            // Enable dynamic payload length on pipe 0
            rf_set_dynpd(DATA_PIPE_0);
            rf_set_ce();
        }
        else if (bind_state == BIND_STATE_8CH) {
            bind_state = BIND_STATE_4CH_1;

            rf_clear_ce();
            // Set special address 12h 23h 23h 45h 78h
            rf_set_rx_address(0, ADDRESS_WIDTH, BIND_ADDRESS);
            // Set special channel 0x51
            rf_set_channel(BIND_CHANNEL);

            rf_set_data_rate(DATA_RATE_250K);
            rf_set_payload_size(DATA_PIPE_0, PAYLOAD_SIZE);
            // Disable dynamic payout size on all pipes
            rf_set_dynpd(0);
            // Disable all dynamic features
            rf_set_feature(0);
            rf_set_ce();
        }
    }


    // ================================
    if (!rf_int_fired) {
        return;
    }
    rf_int_fired = false;

    while (!rf_is_rx_fifo_emtpy()) {
        payload_width = rf_read_payload_width();
        rf_read_fifo(payload, payload_width);
    }
    rf_clear_irq(RX_RD);

    switch (bind_state) {
        case BIND_STATE_4CH_1:
            if (payload[0] == 0xff) {
                if (((payload[1] == 0xaa) && (payload[2] == 0x55)) ||
                    ((payload[1] == 0xab) && (payload[2] == 0x56))) {

                    // Save the protocol identifier (PROTOCOL_3CH=0xaa or PROTOCOL_4CH=0xab)
                    bind_storage_area[PROTOCOLID_INDEX] = payload[1];

                    checksum = 0;
                    for (i = 0; i < 5; i++) {
                        uint8_t payload_byte;

                        payload_byte = payload[3 + i];
                        bind_storage_area[i] = payload_byte;
                        checksum += payload_byte;
                    }
                    bind_state = BIND_STATE_4CH_2;
                }
            }
            break;

        case BIND_STATE_4CH_2:
            if (payload[0] == (checksum & 0xff)) {
                if (payload[1] == (checksum >> 8)) {
                    if (payload[2] == 0) {
                        for (i = 0; i < 7; i++) {
                            bind_storage_area[5 + i] = payload[3 + i];
                        }
                        bind_state = BIND_STATE_4CH_3;
                    }
                }
            }
            break;

        case BIND_STATE_4CH_3:
            if (payload[0] == (checksum & 0xff)) {
                if (payload[1] == (checksum >> 8)) {
                    if (payload[2] == 1) {
                        for (i = 0; i < 7; i++) {
                            bind_storage_area[12 + i] = payload[3 + i];
                        }
                        bind_state = BIND_STATE_4CH_4;
                    }
                }
            }
            break;

        case BIND_STATE_4CH_4:
            if (payload[0] == (checksum & 0xff)) {
                if (payload[1] == (checksum >> 8)) {
                    if (payload[2] == 2) {
                        for (i = 0; i < 6; i++) {
                            bind_storage_area[19 + i] = payload[3 + i];
                        }

                        save_persistent_storage(bind_storage_area);
                        parse_bind_data();
#ifndef NO_DEBUG
                        if (rx_protocol == PROTOCOL_3CH) {
                            uart0_send_cstring("Bind successful (3ch)\n");
                        }
                        else {
                            uart0_send_cstring("Bind successful (4ch)\n");
                        }
#endif
                        binding_done();
                        return;
                    }
                }
            }
            break;

        case BIND_STATE_8CH:
            if (payload_width == 27) {
                if ((payload[0] == 0xac) && (payload[1] == 0x57)) {
                    for (i = 0; i < 25; i++) {
                        bind_storage_area[i] = payload[2 + i];
                    }
                    save_persistent_storage(bind_storage_area);
                    parse_bind_data();
#ifndef NO_DEBUG
                    uart0_send_cstring("Bind successful (8ch)\n");
#endif
                    binding_done();
                }
            }
            break;

        default:
            bind_state = BIND_STATE_4CH_1;
            break;
    }
}


// ****************************************************************************
static void process_4ch_receiving(void)
{
    while (!rf_is_rx_fifo_emtpy()) {
        rf_read_fifo(payload, PAYLOAD_SIZE);
    }
    rf_clear_irq(RX_RD);

#ifndef NO_DEBUG
    if (hops_without_packet > 1) {
        uart0_send_uint32(hops_without_packet);
        uart0_send_linefeed();
    }
#endif

    restart_hop_timer();


    // ================================
    // payload[7] is 0x55 for stick data
    if (payload[7] == stickdata_packetid) {
        channels[0] = stickdata2timer((payload[1] << 8) + payload[0]);
        channels[1] = stickdata2timer((payload[3] << 8) + payload[2]);
        channels[2] = stickdata2timer((payload[5] << 8) + payload[4]);
        channels[3] = stickdata2timer((payload[9] << 8) + payload[6]);
        output_pulses();

        // Save raw received data for the pre-processor to output, so someone
        // can build custom extension based on hijacking channel 3 and using
        // the unused payload bytes 6 and 9.
        // Note:
        //   - See hk310-expansion project for hijacking channel 3
        //   - Custom nRF module firmware required in the transmitter to utilize
        //     payload 6 + 9
        raw_data[0] = stickdata2txdata((payload[5] << 8) + payload[4]);
        raw_data[1] = (payload[6] << 8) + payload[9];


        if (!successful_stick_data) {
            LPC_SCT->CTRL_H &= ~(1u << 2);      // Start the SCTimer H
        }
        successful_stick_data = true;

        failsafe_timer = FAILSAFE_TIMEOUT;
        led_state = LED_STATE_RECEIVING;
    }
    // ================================
    // payload[7] is 0xaa for failsafe data
    else if (payload[7] == failsafe_packetid) {
        // payload[8]: 0x5a if enabled, 0x5b if disabled
        if (payload[8] == 0x5a) {
            failsafe_enabled = true;
            failsafe[0] = stickdata2timer((payload[1] << 8) + payload[0]);
            failsafe[1] = stickdata2timer((payload[3] << 8) + payload[2]);
            failsafe[2] = stickdata2timer((payload[5] << 8) + payload[4]);
            failsafe[3] = stickdata2timer((payload[9] << 8) + payload[6]);
        }
        else {
            // If failsafe is disabled use default values of 1500ms, just
            // like the HKR3000 and XR3100 do.
            initialize_failsafe();
        }
    }
}


// ****************************************************************************
static void process_8ch_receiving(void)
{
    uint8_t payload_width = 0;

    while (!rf_is_rx_fifo_emtpy()) {
        payload_width = rf_read_payload_width();
        rf_read_fifo(payload, payload_width);
    }
    rf_clear_irq(RX_RD);

    if (payload_width != 13) {
        return;
    }

#ifndef NO_DEBUG
    if (hops_without_packet > 1) {
        uart0_send_uint32(hops_without_packet);
        uart0_send_linefeed();
    }
#endif

    restart_hop_timer();

    // ================================
    // payload[0] is 0x57 for stick data
    if (payload[0] == stickdata_packetid) {
        channels[0] = stickdata2timer8ch(((payload[9] & 0x0f) << 8) + payload[1]);
        channels[1] = stickdata2timer8ch(((payload[9] & 0xf0) << 4) + payload[2]);
        channels[2] = stickdata2timer8ch(((payload[10] & 0x0f) << 8) + payload[3]);
        channels[3] = stickdata2timer8ch(((payload[10] & 0xf0) << 4) + payload[4]);
        channels[4] = stickdata2timer8ch(((payload[11] & 0x0f) << 8) + payload[5]);
        channels[5] = stickdata2timer8ch(((payload[11] & 0xf0) << 4) + payload[6]);
        channels[6] = stickdata2timer8ch(((payload[12] & 0x0f) << 8) + payload[7]);
        channels[7] = stickdata2timer8ch(((payload[12] & 0xf0) << 4) + payload[8]);

        if (!successful_stick_data) {
            LPC_SCT->CTRL_H &= ~(1u << 2);      // Start the SCTimer H
        }
        successful_stick_data = true;

        failsafe_timer = FAILSAFE_TIMEOUT;
        led_state = LED_STATE_RECEIVING;
    }
    // ================================
    // payload[7] is 0xac for failsafe data
    else if (payload[0] == failsafe_packetid) {
        failsafe_enabled = true;
        failsafe[0] = stickdata2timer8ch(((payload[9] & 0x0f) << 8) + payload[1]);
        failsafe[1] = stickdata2timer8ch(((payload[9] & 0xf0) << 4) + payload[2]);
        failsafe[2] = stickdata2timer8ch(((payload[10] & 0x0f) << 8) + payload[3]);
        failsafe[3] = stickdata2timer8ch(((payload[10] & 0xf0) << 4) + payload[4]);
        failsafe[4] = stickdata2timer8ch(((payload[11] & 0x0f) << 8) + payload[5]);
        failsafe[5] = stickdata2timer8ch(((payload[11] & 0xf0) << 4) + payload[6]);
        failsafe[6] = stickdata2timer8ch(((payload[12] & 0x0f) << 8) + payload[7]);
        failsafe[7] = stickdata2timer8ch(((payload[12] & 0xf0) << 4) + payload[8]);
    }
}

// ****************************************************************************
static void process_receiving(void)
{
    // ================================
    if (binding) {
        return;
    }

    // ================================
    // Process failsafe only if we ever got a successsful stick data payload
    // after reset.
    //
    // This way the servo outputs stay off until we got successful stick
    // data, so the servos do not got to the failsafe point after power up
    // in case the transmitter is not on yet.
    if (successful_stick_data) {
        if (failsafe_timer == 0) {
            uint8_t i;

            for (i = 0; i < NUMBER_OF_CHANNELS; i++) {
                channels[i] = failsafe[i];
            }
            output_pulses();

            led_state = LED_STATE_FAILSAFE;
        }
    }


    // ================================
    if (perform_hop_requested) {
        perform_hop_requested = false;
        ++hops_without_packet;


        if (hops_without_packet > MAX_HOP_WITHOUT_PACKET) {
            restart_packet_receiving();
        }
        else {
            rf_clear_ce();
            hop_index = (hop_index + 1) % NUMBER_OF_HOP_CHANNELS;
            rf_set_channel(hop_data[hop_index]);
            rf_set_ce();
        }
    }


    // ================================
    if (!rf_int_fired) {
        return;
    }
    rf_int_fired = false;

    if (rx_protocol == PROTOCOL_8CH) {
        process_8ch_receiving();
    }
    else {
        process_4ch_receiving();
    }
}


// ****************************************************************************
static void process_systick(void)
{
    if (!systick) {
        return;
    }

    if (failsafe_timer) {
        --failsafe_timer;
    }

    if (bind_timer) {
        --bind_timer;
    }

    if (bind_swap_timer) {
        --bind_swap_timer;
    }

    if (bind_button_timer) {
        --bind_button_timer;
    }

    if (blink_timer) {
        --blink_timer;
    }
}


// ****************************************************************************
static void process_bind_button(void)
{
    static bool isp_timeout_active;
    static bool old_button_state = BUTTON_RELEASED;
    bool new_button_state;


    if (!systick) {
        return;
    }

    new_button_state = LPC_GPIO_PORT->W0[GPIO_BIT_BIND];

    if (isp_timeout_active && (bind_button_timer == 0)) {
        LPC_GPIO_PORT->SET0 = gpio_mask_led;      // LED off
#ifndef NO_DEBUG
        uart0_send_cstring("Launching ISP!\n");
#endif
        invoke_ISP();
        // We should never return here...
    }

    if (new_button_state == old_button_state) {
        return;
    }
    old_button_state = new_button_state;

    if (new_button_state == BUTTON_PRESSED) {
        bind_button_timer = ISP_TIMEOUT;
        isp_timeout_active = true;
    }

    if (new_button_state == BUTTON_RELEASED) {
        isp_timeout_active = false;
        binding_requested = true;
    }
}


// ****************************************************************************
static void process_led(void)
{
    static led_state_t old_led_state = LED_STATE_UNKNOWN;
    static bool blinking;
    static unsigned int blink_timer_reload_value;


    if (blinking) {
        if (blink_timer == 0) {
            blink_timer = blink_timer_reload_value;
            LPC_GPIO_PORT->NOT0 = gpio_mask_led;     // Toggle the LED
        }
    }

    if (led_state == old_led_state) {
        return;
    }
    old_led_state = led_state;

    switch (led_state) {
        case LED_STATE_RECEIVING:
            LPC_GPIO_PORT->CLR0 = gpio_mask_led;  // LED on
            blinking = false;
            break;

        case LED_STATE_BINDING:
            blink_timer_reload_value = BLINK_TIME_BINDING;
            blinking = true;
            break;

        case LED_STATE_UNKNOWN:
        case LED_STATE_IDLE:
        case LED_STATE_FAILSAFE:
        default:
            blink_timer_reload_value = BLINK_TIME_FAILSAFE;
            blinking = true;
            break;
    }
}


// ****************************************************************************
void init_receiver(void)
{
    load_persistent_storage(bind_storage_area);
    parse_bind_data();
    initialize_failsafe();

    rf_enable_clock();
    rf_clear_ce();
    rf_enable_receiver();

    restart_packet_receiving();

    led_state = LED_STATE_IDLE;
}


// ****************************************************************************
void process_receiver(void)
{
    process_systick();
    process_bind_button();
    process_binding();
    process_receiving();
    process_led();
}


// ****************************************************************************
void rf_interrupt_handler(void)
{
    rf_int_fired = true;
}


// ****************************************************************************
void hop_timer_handler(void)
{
    perform_hop_requested = true;
}
