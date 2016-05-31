#pragma once

void LED_systick_callback(void);
void LED_init(void);
void LED_on(void);
void LED_off(void);
void LED_dim(uint8_t percent);
void LED_flashing(void);
