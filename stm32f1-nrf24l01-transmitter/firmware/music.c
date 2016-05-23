#include <stdlib.h>
#include <stdint.h>

#include <sound.h>
#include <music.h>


// Frequencies for each notes in Hz
// Source: http://www.phy.mtu.edu/~suits/notefreqs.html
#define C3 130.81
#define D3 146.83
#define E3 164.81
#define F3 174.61
#define G3 196.00
#define A3 220.00
#define B3 246.94
#define C4 261.63
#define D4 293.66
#define E4 329.63
#define F4 349.23
#define G4 392.00
#define A4 440.00
#define B4 493.88
#define C5 523.25

#define PAUSE 0
#define SONG_END 0xffff


const uint16_t song_startup[] = {
    C4, C4, F4, F4, A4, A4, C5, C5, C5, C5, A4, A4, C5, C5, C5, C5, C5, C5, C5, SONG_END
};

const uint16_t song_activate[] = {
    C4, D4, E4, F4, G4, C4, D4, E4, F4, G4, A4, B4, C5, C5, C5, C5, SONG_END
};

const uint16_t song_deactivate[] = {
    C5, B4, A4, G4, F4, C5, B4, A4, G4, F4, E4, D4, C4, C4, C4, C4, SONG_END
};

static uint16_t const *song_pointer = NULL;
static uint8_t current_note = 0;


// ****************************************************************************
static uint32_t music_callback(void)
{
    ++current_note;
    if (song_pointer[current_note] == SONG_END) {
        return 0;
    }

    sound_set_frequency(song_pointer[current_note], 100);
    return 60;
}


// ****************************************************************************
void music_play(song const *s)
{
    current_note = 0;
    song_pointer = s;

    sound_set_frequency(song_pointer[current_note], 100);
    sound_start(60, music_callback);
}