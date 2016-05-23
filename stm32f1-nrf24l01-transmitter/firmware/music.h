#pragma once

#include <stdint.h>

typedef uint16_t song;

extern const song song_startup[];
extern const song song_activate[];
extern const song song_deactivate[];

void music_play(song const *s);
