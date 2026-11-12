/* relay: the relay on PB0 (through the BC547B stage): the steady lock level and timed pulses */
#ifndef INC_RELAY_H_
#define INC_RELAY_H_

#include <stdbool.h>
#include <stdint.h>

void relay_set(bool on); /* the steady level (the lock state) */

/* Non-blocking: starts a pulse and returns at once; relay_update() ends it */
void relay_pulse(uint32_t ms); /* the relay on for ms, then back to the relay_set() level */
void relay_update(void);       /* call on every superloop pass */

#endif /* INC_RELAY_H_ */
