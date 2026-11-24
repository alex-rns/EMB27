#include "player.h"

#include "buzzer.h"
#include "notes.h"

// The player's whole state: which melody, the next note, and how many ticks the current note has left.
static const note_t *s_melody;
static size_t s_count;
static size_t s_next;
static uint8_t s_ticks_left;
static bool s_playing;

void player_start(const note_t *melody, size_t count)
{
    s_melody = melody;
    s_count = count;
    s_next = 0;
    s_ticks_left = 0;
    s_playing = (count > 0);
}

void player_tick(void)
{
    if (!s_playing) {
        return;
    }
    if (s_ticks_left > 0) { // the current note goes on
        s_ticks_left--;
        return;
    }
    if (s_next >= s_count) { // the melody is over
        buzzer_off();
        s_playing = false;
        return;
    }

    const note_t *note = &s_melody[s_next++];
    if (note->freq_hz == NOTE_REST) {
        buzzer_off();
    } else {
        buzzer_tone(note->freq_hz);
    }
    s_ticks_left = (note->ticks > 0) ? note->ticks - 1 : 0; // this tick is the note's first
}

bool player_is_playing(void)
{
    return s_playing;
}
