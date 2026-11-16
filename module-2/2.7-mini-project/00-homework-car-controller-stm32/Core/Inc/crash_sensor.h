/* crash_sensor: the crash input (PA8, an EXTI interrupt on the falling edge, set up in CubeMX) */
#ifndef INC_CRASH_SENSOR_H_
#define INC_CRASH_SENSOR_H_
#include <stdbool.h>

/* true once per crash, then the flag is cleared */
bool crash_sensor_take(void);

#endif /* INC_CRASH_SENSOR_H_ */
