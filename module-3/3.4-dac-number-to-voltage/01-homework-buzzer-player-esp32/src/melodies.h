#pragma once

#include <stddef.h>

#include "player.h"

typedef struct {
    const char *name;
    const note_t *notes;
    size_t count;
} melody_t;

size_t melodies_count(void);
const melody_t *melodies_get(size_t index); // index 0 .. melodies_count() - 1
