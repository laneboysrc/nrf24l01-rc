#include <stdint.h>
#include <stdbool.h>

#include <platform.h>
#include <rc_receiver.h>
#include <rf.h>
#include <uart0.h>

#define PAYLOAD_SIZE 10
#define ADDRESS_WIDTH 5
#define NUMBER_OF_HOP_CHANNELS 20
#define NUMBER_OF_CHANNELS 4

#define PULSE_1500MS 0xf82f

bool rf_int_fired = false;
bool data_available = false;
bool binding = false;
unsigned int bind_state;
unsigned int receive_state;
unsigned int bind_timeout;
unsigned int blink_timer;
unsigned int hop_index;
unsigned int inc_every_hop_to_20;
unsigned int failsafe_timer;
uint16_t checksum;
uint8_t status;
uint8_t payload[PAYLOAD_SIZE];
uint8_t bind_storage_area[ADDRESS_WIDTH + NUMBER_OF_HOP_CHANNELS];
uint16_t channels[NUMBER_OF_CHANNELS];
uint16_t failsafe[NUMBER_OF_CHANNELS];
uint8_t failsafe_enabled;
uint8_t model_address[ADDRESS_WIDTH];
uint8_t hop_data[NUMBER_OF_HOP_CHANNELS];


const uint8_t BIND_CHANNEL = 0x51;
const uint8_t BIND_ADDRESS[ADDRESS_WIDTH] = {0x12, 0x23, 0x23, 0x45, 0x78};



// ****************************************************************************
// static void stop_output_pulse_timer(void)
// {

// }

// ****************************************************************************
static void start_output_pulse_timer(void)
{

}

// ****************************************************************************
static void set_timer_to_4ms(void)
{

}

// ****************************************************************************
static void save_bind_data(void)
{
    int i;

    for (i = 0; i < ADDRESS_WIDTH; i++) {
        model_address[i] = bind_storage_area[i];
    }

    for (i = 0; i < NUMBER_OF_HOP_CHANNELS; i++) {
        hop_data[i] = bind_storage_area[ADDRESS_WIDTH + i];
    }
}


static void print_payload(void)
{
    int i;
    for (i = 0; i < PAYLOAD_SIZE; i++) {
        uart0_send_uint8_hex(payload[i]);
        uart0_send_char(' ');
    }
    uart0_send_linefeed();
}

// ****************************************************************************
// static void toggle_red_led(void)
// {

// }

// ****************************************************************************
static void green_led_on(void)
{

}

// ****************************************************************************
static void green_led_off(void)
{

}

// ****************************************************************************
static void red_led_on(void)
{

}

// ****************************************************************************
static void red_led_off(void)
{

}


// ****************************************************************************
static void hop_timer_off(void)
{

}


// ****************************************************************************
static void read_bind_data(void)
{
    int i;

    // Dingo:  9ee187e5d52 e

    // XR311:  04bc285afd 02
    // model_address[0] = 0x04;
    // model_address[1] = 0xbc;
    // model_address[2] = 0x28;
    // model_address[3] = 0x5a;
    // model_address[4] = 0xfd;

    // hop_data[0] = 0x02;

    // Bind packets
    // model_address[0] = 0x12;
    // model_address[1] = 0x23;
    // model_address[2] = 0x23;
    // model_address[3] = 0x45;
    // model_address[4] = 0x78;

    // hop_data[0] = 0x51;

    for (i = 1; i < NUMBER_OF_HOP_CHANNELS; i++) {
        hop_data[i] = hop_data[i - 1] + 1;
    }
}


// ****************************************************************************
// The bind process works as follows:
//
// The transmitter regularly sends data on the fixed channel 51h, with address
// 12:23:23:45:78.
// This data is sent at a lower power. All transmitters do that all the time.
//
// The transmitter cycles through 4 packets:
// ff aa 55 a1 a2 a3 a4 a5
// cc cc 00 ha hb hc hd he hf hg
// cc cc 01 hh hi hj hk hl hm hn
// cc cc 02 ho hp hq hr hs ht
//
// ff aa 55 is the special marker for the first byte
// a1..a5 are the 5 address bytes
// cc cc is a 16 byte checksum of bytes a1..a5
// ha..ht are the 20 hop data bytes
//
// a1..a5 are stored in 14h..18h
// ha..ht are stored in x002a..x003d
//
// ****************************************************************************
static void process_binding(void)
{
    int i;

    if (!binding) {
        if (GPIO_BIND) {
            return;
        }

        binding = true;
        bind_state = 0;
        uart0_send_cstring("Starting bind procedure\n");
        rf_clear_ce();
        rf_set_rx_address(0, ADDRESS_WIDTH, BIND_ADDRESS);    // Set special address 12h 23h 23h 45h 78h
        rf_set_channel(BIND_CHANNEL);
        rf_set_ce();
        return;
    }


    if (!rf_int_fired) {
        return;
    }
    rf_int_fired = 0;

    while (!rf_is_rx_fifo_emtpy()) {
        rf_read_fifo(payload, PAYLOAD_SIZE);
    }
    rf_clear_irq(RX_RD);

    switch (bind_state) {
        case 0:
            if (payload[0] == 0xff) {
                if (payload[1] == 0xaa) {
                    if (payload[2] == 0x55) {
                        checksum = 0;
                        for (i = 0; i < 5; i++) {
                            uint8_t payload_byte;

                            payload_byte = payload[3 + i];
                            bind_storage_area[i] = payload_byte;
                            checksum += payload_byte;
                        }
                        bind_state = 1;
                    }
                }
            }
            break;

        case 1:
            if (payload[0] == (checksum & 0xff)) {
                if (payload[1] == (checksum >> 8)) {
                    if (payload[2] == 0) {
                        for (i = 0; i < 7; i++) {
                            bind_storage_area[5 + i] = payload[3 + i];
                        }
                        bind_state = 2;
                    }
                }
            }
            break;

        case 2:
            if (payload[0] == (checksum & 0xff)) {
                if (payload[1] == (checksum >> 8)) {
                    if (payload[2] == 1) {
                        for (i = 0; i < 7; i++) {
                            bind_storage_area[12 + i] = payload[3 + i];
                        }
                        bind_state = 3;
                    }
                }
            }
            break;

        case 3:
            if (payload[0] == (checksum & 0xff)) {
                if (payload[1] == (checksum >> 8)) {
                    if (payload[2] == 2) {
                        for (i = 0; i < 6; i++) {
                            bind_storage_area[19 + i] = payload[3 + i];
                        }

                        save_bind_data();

                        rf_clear_ce();

                        hop_index = 0;
                        rf_set_rx_address(DATA_PIPE_0, ADDRESS_WIDTH, model_address);
                        rf_set_channel(hop_data[0]);

                        rf_set_ce();
                        binding = false;
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
    if (binding) {
        return;
    }

    if (rf_int_fired) {
        rf_int_fired = 0;
        rf_clear_ce();
        while (!rf_is_rx_fifo_emtpy()) {
            rf_read_fifo(payload, PAYLOAD_SIZE);
        }

        hop_index = (hop_index + 1) % NUMBER_OF_HOP_CHANNELS;
        rf_set_channel(hop_data[hop_index]);

        rf_clear_irq(RX_RD);
        data_available = 1;
        start_output_pulse_timer();

        rf_set_ce();
    }


    // Hex-dump of received payload
    if (data_available) {

        data_available = 0;
        // if (payload[7] == 0xaa) { // F/S; payload[8] = 0x5a if enabled, 0x5b if disabled
        if (payload[7] == 0x55) {   // Stick data
            print_payload();
        }
    }





    // FIXME: remove and fully implement
    return;



    switch (receive_state) {
        case 0:
            rf_set_channel(hop_data[0]);
            receive_state = 1;
            rf_set_ce();
            break;

        case 1:
            if (data_available) {
                hop_index = 0;
                set_timer_to_4ms();
                receive_state = 2;
            }
            break;

        case 2:
            if (data_available) {
                set_timer_to_4ms();
                inc_every_hop_to_20 = 0;
                rf_clear_ce();
                failsafe_timer = 0;
            }
            break;

        default:
            break;
    }

    if (inc_every_hop_to_20 > 16) {
        receive_state = 0;
        hop_timer_off();
    }

    if (data_available) {
        data_available = false;
        if (payload[7] == 0x55) {
            channels[0] = (payload[0] << 8) + payload[1];
            channels[1] = (payload[2] << 8) + payload[3];
            channels[2] = (payload[4] << 8) + payload[5];
            channels[3] = (payload[6] << 8) + payload[7];
        }
        else if (payload[7] == 0xaa) {
            failsafe_enabled = payload[8];
            failsafe[0] = (payload[0] << 8) + payload[1];
            failsafe[1] = (payload[2] << 8) + payload[3];

        }
    }

    if (failsafe_timer < 640) {
        green_led_on();
        red_led_off();
    }
    else {
        green_led_off();
        red_led_on();
        if (failsafe_enabled == 0x5a) {
            channels[0] = failsafe[0];
            channels[1] = failsafe[1];
        }
        else {
            channels[0] = PULSE_1500MS;
            channels[1] = PULSE_1500MS;
        }
    }
}


// ****************************************************************************
void init_receiver(void)
{
    // FIXME: need a delay after power on!

    read_bind_data();

    rf_enable_clock();
    rf_set_crc(CRC_2_BYTES);
    rf_set_irq_source(RX_RD);
    rf_set_data_rate(DATA_RATE_250K);
    rf_set_data_pipes(DATA_PIPE_0, NO_AUTO_ACKNOWLEDGE);
    rf_set_address_width(ADDRESS_WIDTH);
    rf_set_payload_size(DATA_PIPE_0, PAYLOAD_SIZE);
    rf_set_rx_address(DATA_PIPE_0, ADDRESS_WIDTH, model_address);
    rf_set_channel(hop_data[0]);
    rf_enable_receiver();
    rf_flush_rx_fifo();
    rf_clear_irq(RX_RD);

    rf_set_ce();
}


// ****************************************************************************
void process_receiver(void)
{
    process_binding();
    process_receiving();
}


// ****************************************************************************
void rf_interrupt_handler(void)
{
    rf_int_fired = true;
}
