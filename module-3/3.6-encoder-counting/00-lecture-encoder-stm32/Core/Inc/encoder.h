#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

/* Configured encoder resolution in X4 counts per revolution. */
#define ENCODER_DEFAULT_CPR       80U
#define ENCODER_SAMPLE_PERIOD_MS  100U
#define ENCODER_EMA_ALPHA_SPEED         0.25f
#define ENCODER_WHEEL_DIAMETER_M  0.20f

typedef struct
{
  uint32_t cpr; // Counts per revolution

  uint16_t counter; // Prev TIM3 CNT
  int32_t delta_counts; // diff for the last ENCODER_SAMPLE_PERIOD_MS
  int64_t position_counts;
  int32_t direction;             /* +1, -1 or 0 for the last sample window. */
  uint32_t scale_valid;          /* Angle, RPM and distance require cpr > 0. */
  uint32_t elapsed_ms;
  float angle_deg;               /* Signed angle from the software zero. */
  float rpm_raw;
  float rpm_filtered;
  float distance_m;              /* Signed displacement, not total path. */

  /* Sampling bookkeeping; inspect these but do not edit them. */
  uint32_t last_sample_ms;
} EncoderState;

void Encoder_Init(volatile EncoderState *state, uint16_t counter,
                  uint32_t now_ms);
void Encoder_Update(volatile EncoderState *state, uint16_t counter,
                    uint32_t now_ms);

#endif /* ENCODER_H */
