#pragma once

#include <stdint.h>

void init_sound(void);
void sound_play(unsigned int frequency, uint32_t duration_ms, uint8_t volume, void(* cb)(void));
void sound_stop(void);
