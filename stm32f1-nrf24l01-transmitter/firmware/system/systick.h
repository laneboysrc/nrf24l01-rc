#pragma once

#include <stdint.h>


typedef void (* systick_callback)(void);

extern volatile uint32_t milliseconds;


void SYSTICK_init(void);
void SYSTICK_set_callback(systick_callback cb, uint32_t duration_ms);
void SYSTICK_clear_callback(systick_callback cb);
void SYSTICK_set_rf_callback(systick_callback cb, uint32_t repetition_time_ms);
