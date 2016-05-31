#pragma once

#include <stdint.h>


// ****************************************************************************
// Frequencies for each note in Hz
// Source: http://www.phy.mtu.edu/~suits/notefreqs.html
#define C3 131
#define D3 147
#define E3 165
#define F3 175
#define G3 196
#define A3 220
#define B3 247
#define C4 262
#define D4 294
#define E4 330
#define F4 349
#define G4 392
#define A4 440
#define B4 494
#define C5 523
#define D5 587
#define E5 659
#define F5 698
#define G5 784
#define A5 880
#define B5 988
#define C6 1047
#define D6 1175
#define E6 1319
#define F6 1397
#define G6 1568
#define A6 1760
#define B6 1976
#define C7 2093
#define D7 2349
#define E7 2637
#define F7 2794
#define G7 3136
#define A7 3520
#define B7 3951
#define C8 4186
#define D8 4699
#define E8 5274
#define F8 5588
#define G8 6272
#define A8 7040
#define B8 7902


void SOUND_init(void);
void SOUND_set_volume(uint8_t volume);
void SOUND_play(unsigned int frequency, uint32_t duration_ms, void(* cb)(void));
void SOUND_stop(void);
