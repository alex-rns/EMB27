/* app: the car's logic (the glue between the buttons, the sensors and the loads) and the superloop's time slots.
   main.c only calls these two: CubeMX's code stays CubeMX's, ours lives here */
#ifndef INC_APP_H_
#define INC_APP_H_

void app_init(void); /* once, after CubeMX's MX_..._Init() calls */
void app_loop(void); /* on every pass of main()'s while (1); never blocks */

#endif /* INC_APP_H_ */
