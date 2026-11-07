/* STM32F411 / STM32CubeMX-generated project integration example.
 * Reuse the same portable button_fsm.c/.h from esp-idf/main.
 * CubeMX: PB5 INPUT PULLDOWN, pushbutton PB5 -> 3V3;
 * PB6 GPIO OUTPUT, PB6 -> 330R -> LED -> GND.
 * app_button_update() called frequently from while(1).
 * This is an INTEGRATION SNIPPET, not standalone main.c.
 */
#include "main.h"
#include "button_fsm.h"


typedef struct
{
    uint32_t short_press;
    uint32_t long_press;
    uint32_t double_press;
    uint32_t stuck_press;
} press_counts_t;


press_counts_t counters;

static button_fsm_t s_button;
static bool s_led;
static void report(button_event_t ev, void *ctx) {
    (void)ctx;
    if (ev == BUTTON_SHORT) s_led = !s_led;
    else if (ev == BUTTON_DOUBLE || ev == BUTTON_STUCK) s_led = false;
    else if (ev == BUTTON_LONG) s_led = true;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, s_led ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
void app_button_init(void) { button_fsm_init(&s_button, (int64_t)HAL_GetTick()*1000); }
void app_button_update(void) {
    const bool pressed = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET;
    button_fsm_poll(&s_button, pressed, (int64_t)HAL_GetTick()*1000, report, (void*)&counters);
    HAL_Delay(10);
}

