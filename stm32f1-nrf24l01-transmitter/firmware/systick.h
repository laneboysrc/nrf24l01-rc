#pragma once

#include <stdint.h>


typedef void (* systick_callback)(void);

extern volatile uint32_t milliseconds;


void init_systick(void);
void systick_set_callback(systick_callback cb, uint32_t duration_ms);
void systick_clear_callback(systick_callback cb);
