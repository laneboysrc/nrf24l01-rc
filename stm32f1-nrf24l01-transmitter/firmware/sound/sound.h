#pragma once

#include <stdint.h>

void SOUND_init(void);
void SOUND_set_volume(uint8_t volume);
void SOUND_play(unsigned int frequency, uint32_t duration_ms, void(* cb)(void));
void SOUND_stop(void);
