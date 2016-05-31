#pragma once

#include <stdint.h>

typedef struct {
    uint16_t frequency;
    uint16_t duration_ms;
} tone;

typedef struct {
    uint8_t volume;
    const tone *tones;
} song;

extern const song song_startup;
extern const song song_activate;
extern const song song_deactivate;
extern const song song_shutdown;
extern const song song_alarm1;
extern const song song_alarm2;
extern const song song_battery_alarm;

void MUSIC_play(song const *s);
