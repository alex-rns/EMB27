#include <Arduino.h>
#define FLOAT_IN_PIN 45


void setup()
{
    Serial.begin(115200);
    pinMode(FLOAT_IN_PIN, INPUT_PULLUP);

}

void loop()
{

  bool pin_val = digitalRead(FLOAT_IN_PIN);
  Serial.println(pin_val);
  delay(100);
}
