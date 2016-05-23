#pragma once

#include <stdint.h>

void init_sound(void);
void sound_set_frequency(unsigned int frequency, uint8_t volume);
void sound_start(uint32_t duration_ms, uint32_t(* next_note_cb)(void));
void sound_stop(void);
