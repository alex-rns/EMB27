#include "driver/gpio.h"
#include "esp_attr.h"   // IRAM_ATTR
#include  <stdio.h>

#define BUTTON_PIN GPIO_NUM_7  // button to GND, external 10 kOhm pull-up to 3V3

// Written by the ISR, read by the loop: volatile makes the loop re-read it from RAM every time
static volatile uint32_t counter = 0;

// Runs on every falling edge of BUTTON_PIN; IRAM_ATTR keeps it in RAM, not in flash
IRAM_ATTR void button_isr(void *arg) {
    counter++;
}

void app_main(void)
{
    gpio_reset_pin(BUTTON_PIN);                      // also turns the internal pull-up on
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_FLOATING);   // internal pull-up off: only the external 10 kOhm

    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);  // falling edge: 1 -> 0
    gpio_install_isr_service(0);                         // once per program; must come before handler_add
    gpio_isr_handler_add(BUTTON_PIN, button_isr, NULL);  // the function itself, no (); NULL arrives as arg

    static uint32_t last_counter = 0;

    while (true)
    {
        uint32_t now_counter = counter; // read once: the ISR may change counter at any moment

        if (now_counter != last_counter)
        {
            printf("count: %lu\n", now_counter); // uint32_t is unsigned long here: %lu
            last_counter = now_counter;
        }
    }
}
