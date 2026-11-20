/* hysteresis: an on/off decision with two thresholds, so a value wobbling near one threshold can't flip it back and
   forth (no HAL, no pins: it only compares numbers) */
#ifndef INC_HYSTERESIS_H_
#define INC_HYSTERESIS_H_

#include <stdbool.h>
#include <stdint.h>

/* One decision. Its fields are here, not hidden in hysteresis.c, because the caller allocates it (a static variable,
   no malloc); only hysteresis.c touches them. */
typedef struct {
	uint16_t on_below;  /* a value below this switches on */
	uint16_t off_above; /* a value above this switches off; between the two the decision stays as it was */
	bool on;            /* the current decision */
} hysteresis_t;

void hysteresis_init(hysteresis_t *h, uint16_t on_below, uint16_t off_above, bool on); /* on_below < off_above */
bool hysteresis_update(hysteresis_t *h, uint16_t value); /* feed the newest value, get the decision (true = on) */

#endif /* INC_HYSTERESIS_H_ */
