#include <Arduino.h>

#define BUTTON_IN 16  // Button connected to GPIO 16

volatile uint32_t button_counter = 0;  // volatile is required for variables changed inside an ISR

uint32_t last_count = 0; // Tracks changes in the main loop

// Fast interrupt service routine (ISR), placed in RAM (IRAM)
void IRAM_ATTR button_isr() {
  button_counter++;
}

void setup() {
  pinMode(BUTTON_IN, INPUT_PULLUP);
  Serial.begin(115200);

  // Attach the interrupt. FALLING means it fires on a press (a 1 to 0 transition)
  attachInterrupt(digitalPinToInterrupt(BUTTON_IN), button_isr, FALLING);
}

void loop() {
  if (button_counter != last_count) {
    last_count = button_counter;
    Serial.print("Button Pressed! Count: ");
    Serial.println(last_count);
  }

  delay(10);
}
