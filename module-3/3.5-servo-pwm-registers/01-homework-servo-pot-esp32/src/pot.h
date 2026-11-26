#pragma once

// A potentiometer as a divider between 3V3 and GND, its wiper on one ADC1 channel.
void pot_init(void);
int pot_read_mv(void); // the wiper's voltage in millivolts, calibrated, several readings averaged
