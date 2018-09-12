#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <nrf_drv_config.h>
#include <nrf_gpio.h>
#include <nrf_drv_timer.h>
#include <micro_esb.h>
#include <uesb_error_codes.h>
#include <app_util_platform.h>

#include <rc_receiver.h>
#include <persistent_storage.h>


extern volatile uint32_t milliseconds;


#define __SYSTICK_IN_MS 10
#define SERVO_PULSE_CENTER 2048     // Half value of 12 bit

#define NUMBER_OF_CHANNELS 8

#define PAYLOAD_SIZE 10
#define ADDRESS_WIDTH 5
#define NUMBER_OF_HOP_CHANNELS 20
#define MAX_HOP_WITHOUT_PACKET 15
#define FIRST_HOP_TIME_IN_US 2500
#define HOP_TIME_IN_US 5000

#define FAILSAFE_TIMEOUT (640 / __SYSTICK_IN_MS)
#define BIND_TIMEOUT (5000 / __SYSTICK_IN_MS)
// #define ISP_TIMEOUT (3000 / __SYSTICK_IN_MS)
#define BLINK_TIME_FAILSAFE (320 / __SYSTICK_IN_MS)
#define BLINK_TIME_BINDING (50 / __SYSTICK_IN_MS)

#define LED_STATE_IDLE 0
#define LED_STATE_RECEIVING 1
#define LED_STATE_FAILSAFE 2
#define LED_STATE_BINDING 3

#define BUTTON_PRESSED 0
#define BUTTON_RELEASED 1



uint16_t channels[NUMBER_OF_CHANNELS];
uint16_t raw_data[2];
bool successful_stick_data = false;


// static bool rf_int_fired = false;
static unsigned int led_state;
static unsigned int blink_timer;
// static unsigned int bind_button_timer;

// static uint8_t payload[PAYLOAD_SIZE];
static uesb_payload_t payload;

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
static const uint8_t BIND_CHANNEL = 0x51;
static const uint8_t BIND_ADDRESS[ADDRESS_WIDTH] = {0x12, 0x23, 0x23, 0x45, 0x78};
static uint8_t bind_storage_area[ADDRESS_WIDTH + NUMBER_OF_HOP_CHANNELS] __attribute__ ((aligned (4)));


// We are using TIMER1 for the hop timer
static const nrf_drv_timer_t hop_timer = NRF_DRV_TIMER_INSTANCE(1);


// ****************************************************************************
static void print_payload(void)
{
    int i;
    for (i = 0; i < PAYLOAD_SIZE; i++) {
        printf("%02x ", payload.data[i]);
    }
    printf("\n");
}


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
    // int i;

    // for (i = 0; i < NUMBER_OF_CHANNELS; i++) {
    //     LPC_SCT->MATCHREL[i + 1].H = channels[i];
    // }
}


// ****************************************************************************
static uint16_t stickdata2ms(uint16_t stickdata)
{
    uint32_t ms;

    // ms = (0xffff - stickdata) * 3 / 4;
    ms = (0xffff - stickdata);
    return ms & 0xffff;
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
    // LPC_SCT->CTRL_L |= (1 << 2);
    nrf_drv_timer_disable(&hop_timer);

    perform_hop_requested = false;
}


// ****************************************************************************
static void restart_hop_timer(void)
{
    // LPC_SCT->CTRL_L |= (1 << 2);
    // LPC_SCT->MATCHREL[0].L = HOP_TIME_IN_US - 1;

    // // We need to set the MATCH register, not the MATCHREL register here as
    // // only after the first match the MATCHREL gets copied in!
    // LPC_SCT->MATCH[0].L = FIRST_HOP_TIME_IN_US;

    // LPC_SCT->COUNT_L = 0;
    // LPC_SCT->CTRL_L &= ~(1 << 2);


    // FIXME: deal with FIRST_HOP_TIME_IN_US    !!!!!!!!!!!!!!!!!!!!!!!
    // FIXME: deal with FIRST_HOP_TIME_IN_US    !!!!!!!!!!!!!!!!!!!!!!!
    // FIXME: deal with FIRST_HOP_TIME_IN_US    !!!!!!!!!!!!!!!!!!!!!!!
    // FIXME: deal with FIRST_HOP_TIME_IN_US    !!!!!!!!!!!!!!!!!!!!!!!
    // FIXME: deal with FIRST_HOP_TIME_IN_US    !!!!!!!!!!!!!!!!!!!!!!!
    // FIXME: deal with FIRST_HOP_TIME_IN_US    !!!!!!!!!!!!!!!!!!!!!!!
    // FIXME: deal with FIRST_HOP_TIME_IN_US    !!!!!!!!!!!!!!!!!!!!!!!

    nrf_drv_timer_extended_compare(&hop_timer, NRF_TIMER_CC_CHANNEL0, HOP_TIME_IN_US-1, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    nrf_drv_timer_clear(&hop_timer);
    nrf_drv_timer_enable(&hop_timer);


    hops_without_packet = 0;
    perform_hop_requested = false;
}


// ****************************************************************************
static void restart_packet_receiving(void)
{
    stop_hop_timer();

    hop_index = 0;
    hops_without_packet = 0;
    perform_hop_requested = false;

    uesb_stop_rx();
    uesb_set_address(UESB_ADDRESS_PIPE0, model_address);
    uesb_set_rf_channel(hop_data[0]);
    uesb_flush_rx();
    uesb_start_rx();

    // rf_clear_ce();
    // rf_set_rx_address(DATA_PIPE_0, ADDRESS_WIDTH, model_address);
    // rf_set_channel(hop_data[0]);
    // rf_flush_rx_fifo();
    // rf_clear_irq(RX_RD);
    // rf_set_ce();

    // rf_int_fired = false;
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
    static unsigned int bind_state;
    static uint16_t checksum;
    int i;

    // ================================
    if (!binding) {
        if (!binding_requested) {
            return;
        }

        binding_requested = false;
        led_state = LED_STATE_BINDING;
        binding = true;
        bind_state = 0;
        bind_timer = BIND_TIMEOUT;

#ifndef NO_DEBUG
        printf("Starting bind procedure\n");
#endif

        // rf_clear_ce();
        // Set special address 12h 23h 23h 45h 78h
        // rf_set_rx_address(0, ADDRESS_WIDTH, BIND_ADDRESS);
        // Set special channel 0x51
        // rf_set_channel(BIND_CHANNEL);
        // rf_set_ce();

        uesb_stop_rx();
        // Set special address 12h 23h 23h 45h 78h
        uesb_set_address(UESB_ADDRESS_PIPE0, BIND_ADDRESS);
        // Set special channel 0x51
        uesb_set_rf_channel(BIND_CHANNEL);
        uesb_flush_rx();
        uesb_start_rx();

        return;
    }


    // ================================
    if (bind_timer == 0) {
#ifndef NO_DEBUG
        printf("Bind timeout\n");
#endif
        binding_done();
        return;
    }


    // ================================
    // if (!rf_int_fired) {
    //     return;
    // }
    // rf_int_fired = false;

    // while (!rf_is_rx_fifo_emtpy()) {
    //     rf_read_fifo(payload, PAYLOAD_SIZE);
    // }
    // rf_clear_irq(RX_RD);

    if (uesb_read_rx_payload(&payload) != UESB_SUCCESS) {
        return;
    }
    // Flush if more than one packet is pending, use the latest
    while (uesb_read_rx_payload(&payload) == UESB_SUCCESS);

    print_payload();

    switch (bind_state) {
        case 0:
            if (payload.data[0] == 0xff) {
                if (payload.data[1] == 0xaa) {
                    if (payload.data[2] == 0x55) {
                        checksum = 0;
                        for (i = 0; i < 5; i++) {
                            uint8_t payload_byte;

                            payload_byte = payload.data[3 + i];
                            bind_storage_area[i] = payload_byte;
                            checksum += payload_byte;
                        }
                        bind_state = 1;
                    }
                }
            }
            break;

        case 1:
            if (payload.data[0] == (checksum & 0xff)) {
                if (payload.data[1] == (checksum >> 8)) {
                    if (payload.data[2] == 0) {
                        for (i = 0; i < 7; i++) {
                            bind_storage_area[5 + i] = payload.data[3 + i];
                        }
                        bind_state = 2;
                    }
                }
            }
            break;

        case 2:
            if (payload.data[0] == (checksum & 0xff)) {
                if (payload.data[1] == (checksum >> 8)) {
                    if (payload.data[2] == 1) {
                        for (i = 0; i < 7; i++) {
                            bind_storage_area[12 + i] = payload.data[3 + i];
                        }
                        bind_state = 3;
                    }
                }
            }
            break;

        case 3:
            if (payload.data[0] == (checksum & 0xff)) {
                if (payload.data[1] == (checksum >> 8)) {
                    if (payload.data[2] == 2) {
                        for (i = 0; i < 6; i++) {
                            bind_storage_area[19 + i] = payload.data[3 + i];
                        }

                        save_persistent_storage(bind_storage_area);
                        parse_bind_data();
#ifndef NO_DEBUG
                        printf("Bind successful\n");
#endif
                        binding_done();
                        return;
                    }
                }
            }
            break;

        default:
            bind_state = 0;
            break;
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
            int i;

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
            // rf_clear_ce();
            hop_index = (hop_index + 1) % NUMBER_OF_HOP_CHANNELS;
            // rf_set_channel(hop_data[hop_index]);
            // rf_set_ce();

            uesb_stop_rx();
            uesb_set_rf_channel(hop_data[hop_index]);
            uesb_start_rx();
        }
    }


    // ================================
    // if (!rf_int_fired) {
    //     return;
    // }
    // rf_int_fired = false;

    // while (!rf_is_rx_fifo_emtpy()) {
    //     rf_read_fifo(payload, PAYLOAD_SIZE);
    // }
    // rf_clear_irq(RX_RD);

    if (uesb_read_rx_payload(&payload) != UESB_SUCCESS) {
        return;
    }

    while (uesb_read_rx_payload(&payload) == UESB_SUCCESS);

#ifndef NO_DEBUG
    if (hops_without_packet > 1) {
        printf("%u\n", hops_without_packet);
    }
#endif

    restart_hop_timer();


    // ================================
    // payload[7] is 0x55 for stick data
    if (payload.data[7] == 0x55) {   // Stick data
        channels[0] = stickdata2ms((payload.data[1] << 8) + payload.data[0]);
        channels[1] = stickdata2ms((payload.data[3] << 8) + payload.data[2]);
        channels[2] = stickdata2ms((payload.data[5] << 8) + payload.data[4]);
        output_pulses();

        // Save raw received data for the pre-processor to output, so someone
        // can build custom extension based on hijacking channel 3 and using
        // the unused payload bytes 6 and 9.
        // Note:
        //   - See hk310-expansion project for hijacking channel 3
        //   - Custom nRF module firmware required in the transmitter to utilize
        //     payload 6 + 9
        raw_data[0] = stickdata2txdata((payload.data[5] << 8) + payload.data[4]);
        raw_data[1] = (payload.data[6] << 8) + payload.data[9];


        if (!successful_stick_data) {
            // LPC_SCT->CTRL_H &= ~(1u << 2);      // Start the SCTimer H
        }
        successful_stick_data = true;

        failsafe_timer = FAILSAFE_TIMEOUT;
        led_state = LED_STATE_RECEIVING;
    }
    // ================================
    // payload[7] is 0xaa for failsafe data
    else if (payload.data[7] == 0xaa) {
        // payload[8]: 0x5a if enabled, 0x5b if disabled
        if (payload.data[8] == 0x5a) {
            failsafe_enabled = true;
            failsafe[0] = stickdata2ms((payload.data[1] << 8) + payload.data[0]);
            failsafe[1] = stickdata2ms((payload.data[3] << 8) + payload.data[2]);
            failsafe[2] = stickdata2ms((payload.data[5] << 8) + payload.data[4]);
        }
        else {
            // If failsafe is disabled use default values of 1500ms, just
            // like the HKR3000 and XR3100 do.
            initialize_failsafe();
        }
    }
}


// ****************************************************************************
static void process_systick(void)
{
    if (failsafe_timer) {
        --failsafe_timer;
    }

    if (bind_timer) {
        --bind_timer;
    }

    // if (bind_button_timer) {
    //     --bind_button_timer;
    // }

    if (blink_timer) {
        --blink_timer;
    }
}


// ****************************************************************************
static void process_bind_button(void)
{
    static bool old_button_state = BUTTON_RELEASED;
    bool new_button_state;


    new_button_state = nrf_gpio_pin_read(GPIO_BIND);


    if (new_button_state == old_button_state) {
        return;
    }
    old_button_state = new_button_state;

    // if (new_button_state == BUTTON_PRESSED) {
    //     bind_button_timer = ISP_TIMEOUT;
    // }

    if (new_button_state == BUTTON_RELEASED) {
        binding_requested = true;
    }
}


// ****************************************************************************
static void process_led(void)
{
    static unsigned int old_led_state = 0xffffffff;
    static bool blinking;
    static unsigned int blink_timer_reload_value;


    if (blinking) {
        if (blink_timer == 0) {
            blink_timer = blink_timer_reload_value;
            nrf_gpio_pin_toggle(GPIO_LED);
        }
    }

    if (led_state == old_led_state) {
        return;
    }
    old_led_state = led_state;

    nrf_gpio_pin_clear(GPIO_LED);

    switch (led_state) {
        case LED_STATE_RECEIVING:
            blinking = false;
            break;

        case LED_STATE_BINDING:
            blink_timer_reload_value = BLINK_TIME_BINDING;
            blinking = true;
            break;

        case LED_STATE_IDLE:
        case LED_STATE_FAILSAFE:
        default:
            blink_timer_reload_value = BLINK_TIME_FAILSAFE;
            blinking = true;
            break;
    }
}


// ****************************************************************************
static void hop_timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    perform_hop_requested = true;
}


// ****************************************************************************
void RECEIVER_init(void)
{
    uesb_config_t uesb_config = UESB_DEFAULT_CONFIG;

    nrf_drv_timer_config_t timer_config  = {
        .frequency          = NRF_TIMER_FREQ_1MHz,
        .mode               = NRF_TIMER_MODE_TIMER,
        .bit_width          = NRF_TIMER_BIT_WIDTH_16,
        .interrupt_priority = APP_IRQ_PRIORITY_LOW,
        .p_context          = NULL,
    };

    load_persistent_storage(bind_storage_area);
    parse_bind_data();
    initialize_failsafe();

    nrf_drv_timer_init(&hop_timer, &timer_config, hop_timer_handler);
    nrf_drv_timer_clear(&hop_timer);

    // rf_enable_clock();
    // rf_clear_ce();
    // rf_enable_receiver();

    // rf_set_crc(CRC_2_BYTES);
    // rf_set_irq_source(RX_RD);
    // rf_set_data_rate(DATA_RATE_250K);
    // rf_set_data_pipes(DATA_PIPE_0, NO_AUTO_ACKNOWLEDGE);
    // rf_set_address_width(ADDRESS_WIDTH);
    // rf_set_payload_size(DATA_PIPE_0, PAYLOAD_SIZE);

    uesb_config.protocol            = UESB_PROTOCOL_ESB;
    uesb_config.bitrate             = UESB_BITRATE_250KBPS;
    uesb_config.mode                = UESB_MODE_PRX;
    uesb_config.crc                 = UESB_CRC_16BIT;
    uesb_config.dynamic_ack_enabled = 0;
    uesb_config.dynamic_payload_length_enabled = 0;
    uesb_config.payload_length      = PAYLOAD_SIZE;
    uesb_config.rf_addr_length      = ADDRESS_WIDTH;
    uesb_config.rx_pipes_enabled    = 0x01;                              \

    uesb_init(&uesb_config);

    restart_packet_receiving();

    led_state = LED_STATE_IDLE;
}


// ****************************************************************************
void RECEIVER_process(void)
{
    static uint32_t next = __SYSTICK_IN_MS;

    if (milliseconds >= next) {
        next += __SYSTICK_IN_MS;
        process_systick();
        process_bind_button();
    }

    process_binding();
    process_receiving();
    process_led();
}


// ****************************************************************************
// void rf_interrupt_handler(void)
// {
//     rf_int_fired = true;
// }

