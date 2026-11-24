#include "buzzer.h"

#define NOTE_A4 440 // Hz

void app_main(void)
{
    buzzer_init();
    buzzer_tone(NOTE_A4); // one steady tone; the LEDC hardware keeps it going after app_main returns
}
