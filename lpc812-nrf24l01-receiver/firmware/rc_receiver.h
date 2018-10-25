#pragma once

typedef enum {
    PROTOCOL_3CH = 0xaa,
    PROTOCOL_4CH = 0xab,
    PROTOCOL_8CH = 0xac,
} rx_protocol_t;

void process_receiver(void);
void init_receiver(void);
void rf_interrupt_handler(void);
void hop_timer_handler(void);
