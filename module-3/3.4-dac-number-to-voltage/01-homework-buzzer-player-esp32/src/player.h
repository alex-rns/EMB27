#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// One note of a melody: a frequency (NOTE_REST = silence) and how many player ticks it lasts.
typedef struct {
    uint16_t freq_hz;
    uint8_t ticks;
} note_t;

void player_start(const note_t *melody, size_t count); // returns at once: the notes play on player_tick()
void player_tick(void);                                 // call it on every fixed tick
bool player_is_playing(void);
