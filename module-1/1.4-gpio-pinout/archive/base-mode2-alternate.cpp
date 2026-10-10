#include <Arduino.h>

#define GREEN_LED_PIN 15
#define BLUE_LED_PIN 16
#define BOOT_BTN 0

void setup()
{
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BOOT_BTN, INPUT);
}

void loop()
{
  int btnState = digitalRead(BOOT_BTN);

  if (btnState == LOW)
  {
    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    delay(1000);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay(1000);
  }
  else
  { 
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
  }    
}
