#include <Arduino.h>


#define LED_PIN 48
#define LED_CHANNEL 0

#define USE_BUILD_IN_RGB (1u)
#define USE_LED_PIN      (0u)
#define USE_PWM          (0u)

void setup()
{
#if USE_LED_PIN == 1u
    pinMode(LED_PIN, OUTPUT);
#endif
#if USE_PWM == 1u
    ledcSetup(LED_CHANNEL, 5000, 8);   // channel, frequency, resolution
    ledcAttachPin(LED_PIN, LED_CHANNEL);
#endif
}

void loop()
{

#if USE_LED_PIN == 1u
    digitalWrite(LED_PIN, HIGH);
    delay(100);

    digitalWrite(LED_PIN, LOW);
    delay(900);
#endif

#if USE_PWM == 1u
    for (int duty = 0; duty <= 255; duty++) {
        ledcWrite(LED_CHANNEL, duty);
        delay(8);
    }

    for (int duty = 255; duty >= 0; duty--) {
        ledcWrite(LED_CHANNEL, duty);
        delay(8);
    }
#endif
#if USE_BUILD_IN_RGB == 1u
    digitalWrite(RGB_BUILTIN, HIGH);   // Turn the RGB LED white
    delay(50);
    digitalWrite(RGB_BUILTIN, LOW);    // Turn the RGB LED off
    delay(50);

    neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,0,0); // Red
    delay(50);
    neopixelWrite(RGB_BUILTIN,0,RGB_BRIGHTNESS,0); // Green
    delay(50);
    neopixelWrite(RGB_BUILTIN,0,0,RGB_BRIGHTNESS); // Blue
    delay(50);
    neopixelWrite(RGB_BUILTIN,0,0,0); // Off / black
    delay(200);
#endif
}
