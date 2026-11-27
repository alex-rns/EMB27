#include "encoder.h"

void Encoder_Init(volatile EncoderState *state, uint16_t counter,
                  uint32_t now_ms)
{
  state->cpr = ENCODER_DEFAULT_CPR;
  state->counter = counter;
  state->delta_counts = 0;
  state->position_counts = 0;
  state->direction = 0;
  state->scale_valid = (state->cpr != 0U);
  state->elapsed_ms = 0U;
  state->angle_deg = 0.0f;
  state->rpm_raw = 0.0f;
  state->rpm_filtered = 0.0f;
  state->distance_m = 0.0f;
  state->last_sample_ms = now_ms;
}

void Encoder_Update(volatile EncoderState *state, uint16_t counter,
                    uint32_t now_ms)
{
  uint32_t elapsed_ms;
  int32_t delta;
  float revolutions;
  float rpm;

  elapsed_ms = now_ms - state->last_sample_ms;
  if (elapsed_ms < ENCODER_SAMPLE_PERIOD_MS)
  {
    return;
  }

  delta = (int16_t)(counter - state->counter);

  state->counter = counter;
  state->delta_counts = delta;
  state->position_counts += delta;
  state->direction = (delta > 0) ? 1 : ((delta < 0) ? -1 : 0);
  state->elapsed_ms = elapsed_ms;
  state->last_sample_ms = now_ms;

	revolutions = (float)state->position_counts / (float)state->cpr;
	rpm = (float)delta * 60000.0f / ((float)state->cpr * (float)elapsed_ms);
	state->angle_deg = revolutions * 360.0f;
	state->distance_m = revolutions * 3.14159265358979323846f * ENCODER_WHEEL_DIAMETER_M;
	state->rpm_raw = rpm;
    state->rpm_filtered += ENCODER_EMA_ALPHA_SPEED * (rpm - state->rpm_filtered);
}
