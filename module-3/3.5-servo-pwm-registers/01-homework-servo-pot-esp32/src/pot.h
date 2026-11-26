#pragma once

// A potentiometer as a divider between 3V3 and GND, its wiper on one ADC1 channel.
void pot_init(void);
float pot_read_deg(void); // the knob's angle from its leftmost position (0 V), 128 readings averaged
