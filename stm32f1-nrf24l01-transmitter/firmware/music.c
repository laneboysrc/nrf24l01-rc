#include <stdlib.h>
#include <stdint.h>

#include <sound.h>
#include <music.h>


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

#define PAUSE 0
#define SONG_END 0xffff


// ****************************************************************************
static const tone song_startup_tones[] = {
        {C4, 80},
        {PAUSE, 40},
        {F4, 80},
        {PAUSE, 40},
        {A4, 80},
        {PAUSE, 40},
        {C5, 160},
        {PAUSE, 40},
        {A4, 80},
        {PAUSE, 40},
        {C5, 240},
        {SONG_END, 0}
};

const song song_startup = {
    .volume = 100,
    .tones = song_startup_tones
};

// ****************************************************************************
static const tone song_activate_tones[] = {
        {C4, 80},
        {D4, 80},
        {E4, 80},
        {F4, 80},
        {G4, 80},
        {C4, 80},
        {D4, 80},
        {E4, 80},
        {F4, 80},
        {G4, 80},
        {A4, 80},
        {B4, 80},
        {C5, 320},
        {SONG_END, 0}
};

const song song_activate = {
    .volume = 100,
    .tones = song_activate_tones
};

// ****************************************************************************
static const tone song_deactivate_tones[] = {
        {C5, 80},
        {B4, 80},
        {A4, 80},
        {G4, 80},
        {F4, 80},
        {C5, 80},
        {B4, 80},
        {A4, 80},
        {G4, 80},
        {F4, 80},
        {E4, 80},
        {D4, 80},
        {C4, 320},
        {SONG_END, 0}
};

const song song_deactivate = {
    .volume = 100,
    .tones = song_deactivate_tones
};


static const song *song_pointer = NULL;
static uint8_t current_tone_index = 0;


static void play_current_tone(void);


// ****************************************************************************
static void music_callback(void)
{
    ++current_tone_index;
    play_current_tone();
}


// ****************************************************************************
static void play_current_tone(void)
{
    const tone *t = &song_pointer->tones[current_tone_index];

    if (t->frequency != SONG_END) {
        sound_play(t->frequency, t->duration_ms, music_callback);
    }
}


// ****************************************************************************
void music_play(song const *s)
{
    song_pointer = s;
    current_tone_index = 0;

    sound_set_volume(s->volume);
    play_current_tone();
}