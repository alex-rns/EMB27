#include "melodies.h"

#include "notes.h"

// Note lengths in player ticks (50 ms each)
#define EIGHTH 4
#define QUARTER 8
#define HALF 16

#define COUNT_OF(a) (sizeof(a) / sizeof((a)[0]))

// The assignment's melodies; each ends with a rest so the next one starts apart.

// Baby Shark: C D E | C D E | C D E
static const note_t s_baby_shark[] = {
    {NOTE_C4, QUARTER}, {NOTE_D4, QUARTER}, {NOTE_E4, QUARTER},
    {NOTE_C4, QUARTER}, {NOTE_D4, QUARTER}, {NOTE_E4, QUARTER},
    {NOTE_C4, QUARTER}, {NOTE_D4, QUARTER}, {NOTE_E4, HALF},
    {NOTE_REST, HALF},
};

// Jingle Bells: E E E | E E E | E G C D E
static const note_t s_jingle_bells[] = {
    {NOTE_E4, QUARTER}, {NOTE_E4, QUARTER}, {NOTE_E4, HALF},
    {NOTE_E4, QUARTER}, {NOTE_E4, QUARTER}, {NOTE_E4, HALF},
    {NOTE_E4, QUARTER}, {NOTE_G4, QUARTER}, {NOTE_C4, QUARTER}, {NOTE_D4, QUARTER}, {NOTE_E4, HALF},
    {NOTE_REST, HALF},
};

// We Will Rock You: C C D | C C D | C C D E (two short stomps and a clap)
static const note_t s_we_will_rock_you[] = {
    {NOTE_C4, EIGHTH}, {NOTE_C4, EIGHTH}, {NOTE_D4, QUARTER},
    {NOTE_C4, EIGHTH}, {NOTE_C4, EIGHTH}, {NOTE_D4, QUARTER},
    {NOTE_C4, EIGHTH}, {NOTE_C4, EIGHTH}, {NOTE_D4, QUARTER}, {NOTE_E4, HALF},
    {NOTE_REST, HALF},
};

// Twinkle Twinkle Little Star: C C G G | A A G
static const note_t s_twinkle[] = {
    {NOTE_C4, QUARTER}, {NOTE_C4, QUARTER}, {NOTE_G4, QUARTER}, {NOTE_G4, QUARTER},
    {NOTE_A4, QUARTER}, {NOTE_A4, QUARTER}, {NOTE_G4, HALF},
    {NOTE_REST, HALF},
};

// Old MacDonald: G G G D E E D
static const note_t s_old_macdonald[] = {
    {NOTE_G4, QUARTER}, {NOTE_G4, QUARTER}, {NOTE_G4, QUARTER}, {NOTE_D4, QUARTER},
    {NOTE_E4, QUARTER}, {NOTE_E4, QUARTER}, {NOTE_D4, HALF},
    {NOTE_REST, HALF},
};

static const melody_t s_melodies[] = {
    {"Baby Shark", s_baby_shark, COUNT_OF(s_baby_shark)},
    {"Jingle Bells", s_jingle_bells, COUNT_OF(s_jingle_bells)},
    {"We Will Rock You", s_we_will_rock_you, COUNT_OF(s_we_will_rock_you)},
    {"Twinkle Twinkle Little Star", s_twinkle, COUNT_OF(s_twinkle)},
    {"Old MacDonald", s_old_macdonald, COUNT_OF(s_old_macdonald)},
};

size_t melodies_count(void)
{
    return COUNT_OF(s_melodies);
}

const melody_t *melodies_get(size_t index)
{
    return &s_melodies[index % COUNT_OF(s_melodies)]; // an index past the end wraps around instead of reading past the array
}
