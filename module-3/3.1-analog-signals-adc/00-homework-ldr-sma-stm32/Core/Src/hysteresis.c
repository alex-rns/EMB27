/* hysteresis: an on/off decision with two thresholds, so a value wobbling near one threshold can't flip it back and
   forth (no HAL, no pins: it only compares numbers) */
#include "hysteresis.h"

void hysteresis_init(hysteresis_t *h, uint16_t on_below, uint16_t off_above, bool on)
{
	h->on_below = on_below;
	h->off_above = off_above;
	h->on = on;
}

bool hysteresis_update(hysteresis_t *h, uint16_t value)
{
	if (value < h->on_below) {
		h->on = true;
	} else if (value > h->off_above) {
		h->on = false;
	}
	/* between the thresholds (and exactly on one of them) nothing changes: the decision remembers the past */
	return h->on;
}
