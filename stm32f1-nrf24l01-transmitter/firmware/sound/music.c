#include <stdint.h>
#include <stdlib.h>

#include <music.h>
#include <sound.h>


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


// ****************************************************************************
static const tone song_shutdown_tones[] = {
        {G6, 100},
        {A5, 100},
        {F6, 100},
        {F5, 100},
        {C6, 100},
        {SONG_END, 0}
};

const song song_shutdown = {
    .volume = 100,
    .tones = song_shutdown_tones
};


// ****************************************************************************
static const tone song_alarm1_tones[] = {
        {D6, 100},
        {D7, 100},
        {SONG_END, 0}
};

const song song_alarm1 = {
    .volume = 100,
    .tones = song_alarm1_tones
};


// ****************************************************************************
static const tone song_alarm2_tones[] = {
        {F7, 100},
        {D6, 100},
        {D7, 100},
        {SONG_END, 0}
};

const song song_alarm2 = {
    .volume = 100,
    .tones = song_alarm2_tones
};


// ****************************************************************************
static const tone song_battery_alarm_tones[] = {
        {C7, 50},
        {PAUSE, 50},
        {F6, 50},
        {PAUSE, 50},
        {C6, 50},
        {SONG_END, 0}
};

const song song_battery_alarm = {
    .volume = 70,
    .tones = song_battery_alarm_tones
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
        SOUND_play(t->frequency, t->duration_ms, music_callback);
    }
}


// ****************************************************************************
void MUSIC_play(song const *s)
{
    song_pointer = s;
    current_tone_index = 0;

    SOUND_set_volume(s->volume);
    play_current_tone();
}