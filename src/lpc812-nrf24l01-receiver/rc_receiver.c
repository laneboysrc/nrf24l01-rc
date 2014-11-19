
#include <rf.h>


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
static void process_binding()
{
    stop_output_pulse_timer();
    rf_int_fired = 0;
    rf_set_address(0, BIND_ADDRESS);    // Set special address 12h 23h 23h 45h 78h
    rf_clear_ce();
    rf_set_channel(BIND_CHANNEL);
    bind_timeout = 0;
    while (bind_timeout < 5000) {
        bind_state = 0;
        rf_set_ce();
        blink_timer = 0;
        if (rf_int_fired) {
            status = rf_get_status();
            while (!rf_is_rx_fifo_emtpy()) {
                rf_read_fifo(&payload);
            }
            rf_int_fired = 0;
            switch (bind_state) {
                case 0:
                    if (payload[0] = 0xff) {
                        if (payload[1] = 0xaa) {
                            if (payload[2] = 0x55) {
                                checksum = 0;
                                for (i = 0; i < 5; i++) {
                                    bind_storage_area[i] = payload[3 + i];
                                    checksum += payload[i];
                                }
                                bind_state = 1;
                            }
                        }
                    }
                    break;

                case 1:
                    if (payload[0] = checksum & 0xff) {
                        if (payload[1] = checksum >> 8) {
                            if (payload[2] = 0) {
                                for (i = 0; i < 7; i++) {
                                    bind_storage_area[5 + i] = payload[3 + i];
                                }
                                bind_state = 2;
                            }
                        }
                    }
                    break;

                case 2:
                    if (payload[0] = checksum & 0xff) {
                        if (payload[1] = checksum >> 8) {
                            if (payload[2] = 1) {
                                for (i = 0; i < 7; i++) {
                                    bind_storage_area[12 + i] = payload[3 + i];
                                }
                                bind_state = 3;
                            }
                        }
                    }
                    break;

                case 3:
                    if (payload[0] = checksum & 0xff) {
                        if (payload[1] = checksum >> 8) {
                            if (payload[2] = 2) {
                                for (i = 0; i < 6; i++) {
                                    bind_storage_area[19 + i] = payload[3 + i];
                                }
                                save_bind_data();
                                return;
                            }
                        }
                    }
                    break;
            }
        }

        delay(TIME_1MS);
        bind_timeout += 1;
        blink_timer += 1;
        if (blink_timer > 1000) {
            blink_timer = 0;
            toggle_red_led();
        }

    }

}


// ****************************************************************************
static void process_receiving()
{
    if (rf_int_fired) {
        status = rf_get_status();
        rf_clear_ce();
        while (!rf_is_rx_fifo_emtpy()) {
            rf_read_fifo(&payload);
        }
        rf_int_fired = 0;
        rf_data_available = 1;
        start_output_pulse_timer();
    }

    switch (receive_state) {
        case 0:
            rf_set_channel(hop_data[0]);
            receive_state = 1;
            rf_set_ce();
            break;

        case 1:
            if (rf_data_available) {
                hop_index = 0;
                set_timer_to_4ms();
                receive_state = 2;
            }
            break;

        case 2:
            if (rf_data_available) {
                set_timer_to_4ms();
                inc_very_hop_to_20 = 0;
                rf_clear_ce();
                failsafe_timer = 0;
            }
            break;
    }

    if (inc_very_hop_to_20 > 16) {
        receive_state = 0;
        timer_off();
    }

    if (rf_data_available) {
        rf_data_available = false;
        if (payload[7] == 0x55) {
            channels[0] = payload[0] << 8 + payload[1];
            channels[1] = payload[2] << 8 + payload[3];
            channels[2] = payload[4] << 8 + payload[5];
            channels[3] = payload[6] << 8 + payload[7];
        }
        else if (payload[7] == 0xaa) {
            failsafe_enabled = payload[8];
            failsafe[0] = payload[0] << 8 + payload[1];
            failsafe[1] = payload[2] << 8 + payload[3];

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
    read_bind_data();
    rf_mask_interrupt(MAX_RT, 0);
    rf_mask_interrupt(TX_DS, 0);
    rf_mask_interrupt(RX_RD, 1);
    rf_init_data_pipes(0);
    rf_setup_address_width(5);
    rf_set_data_rate(250);
    rf_set_address(0, model_address);
    rf_set_payload_size(0, 10);
    rf_set_channel(hop_data[0]);
    rf_enable_receiver();
    rf_power_up();
    rf_set_ce();
}


// ****************************************************************************
void process_receiver(void)
{
    process_binding();
    process_receiving();
}