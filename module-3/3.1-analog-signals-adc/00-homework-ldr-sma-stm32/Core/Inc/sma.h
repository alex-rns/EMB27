/* sma: simple moving average of the latest readings (no HAL, no pins: it only does arithmetic) */
#ifndef INC_SMA_H_
#define INC_SMA_H_

#include <stdbool.h>
#include <stdint.h>

/* One average. Its fields are here, not hidden in sma.c, because the caller allocates it (a static variable, no
   malloc); only sma.c touches them. The samples buffer is the caller's too, so each average picks its own length. */
typedef struct {
	uint16_t *samples; /* ring buffer of the latest readings, len slots */
	uint16_t len;      /* how many readings are averaged */
	uint16_t index;    /* where the next reading goes (the oldest one is there) */
	uint32_t sum;      /* sum of the buffer, kept up to date: up to 65535 x 4095 fits in 32 bits */
	bool primed;       /* the buffer has been filled with the first reading */
} sma_t;

void sma_init(sma_t *sma, uint16_t *samples, uint16_t len); /* empty average over the caller's buffer of len slots */
uint16_t sma_add(sma_t *sma, uint16_t sample);              /* add the newest reading, return the new average */

#endif /* INC_SMA_H_ */
