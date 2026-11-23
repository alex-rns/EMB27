/* sma: simple moving average of the latest readings (no HAL, no pins: it only does arithmetic) */
#include "sma.h"

void sma_init(sma_t *sma, uint16_t *samples, uint16_t len)
{
	sma->samples = samples;
	sma->len = len;
	sma->index = 0u;
	sma->sum = 0u;
	sma->primed = false;
}

uint16_t sma_add(sma_t *sma, uint16_t sample)
{
	/* first reading: fill the whole buffer with it, so the average starts right instead of climbing from 0
	   (for the LDR that would look like darkness until the buffer is full) */
	if (!sma->primed) {
		for (uint16_t i = 0u; i < sma->len; i++) {
			sma->samples[i] = sample;
		}
		sma->sum = (uint32_t)sample * sma->len;
		sma->primed = true;
	}

	/* the newest reading takes the place of the oldest one; the sum changes by their difference */
	sma->sum -= sma->samples[sma->index];
	sma->samples[sma->index] = sample;
	sma->sum += sample;
	sma->index++;
	if (sma->index >= sma->len) {
		sma->index = 0u; /* past the last slot: back to the first */
	}
	return (uint16_t)(sma->sum / sma->len);
}
