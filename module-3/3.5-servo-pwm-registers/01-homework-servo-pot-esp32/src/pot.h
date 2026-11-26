#pragma once

#include <stdint.h>

// A potentiometer as a divider between 3V3 and GND, its wiper on one ADC1 channel.
void pot_init(void);
uint32_t pot_read_deg(void); // the knob's angle from its leftmost position (0 V), in degrees
