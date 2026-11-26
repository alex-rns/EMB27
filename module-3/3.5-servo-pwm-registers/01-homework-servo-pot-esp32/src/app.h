#pragma once

// The homework's logic: the potentiometer sets the servo 1:1, the angle is logged.
void app_init(void);
void app_run(void); // one pass: read the knob, set the servo, log the angle if it changed
