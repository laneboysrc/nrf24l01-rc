#include <stdlib.h>
#include <stdint.h>

#include <sound.h>
#include <music.h>


// Frequencies for each notes in Hz
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

#define PAUSE 0
#define SONG_END 0xffff

const tone song_startup_tones[] = {
        {C4, 100},
        {F4, 100},
        {A4, 100},
        {C5, 200},
        {A4, 100},
        {C5, 300},
        {SONG_END, 0}
};

const song song_startup = {
    .volume = 100,
    .tones = song_startup_tones
};

const tone song_activate_tones[] = {
        {C4, 50},
        {D4, 50},
        {E4, 50},
        {F4, 50},
        {C4, 50},
        {D4, 50},
        {E4, 50},
        {F4, 50},
        {G4, 50},
        {A4, 50},
        {B4, 50},
        {C5, 200},
        {SONG_END, 0}
};

const song song_activate = {
    .volume = 100,
    .tones = song_activate_tones
};

const tone song_deactivate_tones[] = {
        {C5, 50},
        {B4, 50},
        {A4, 50},
        {G4, 50},
        {F4, 50},
        {C5, 50},
        {B4, 50},
        {A4, 50},
        {G4, 50},
        {F4, 50},
        {E4, 50},
        {D4, 50},
        {C4, 200},
        {SONG_END, 0}
};


const song song_deactivate = {
    .volume = 100,
    .tones = song_deactivate_tones
};

static const song *song_pointer = NULL;
static uint8_t current_note = 0;


// ****************************************************************************
static void music_callback(void)
{
    const tone *t;

    ++current_note;
    t = &song_pointer->tones[current_note];

    if (t->frequency == SONG_END) {
        return;
    }

    sound_play(t->frequency, t->duration_ms, song_pointer->volume, music_callback);
}


// ****************************************************************************
void music_play(song const *s)
{
    current_note = 0;
    song_pointer = s;

    sound_play(song_pointer->tones[0].frequency,
               song_pointer->tones[0].duration_ms,
               song_pointer->volume,
               music_callback);
}