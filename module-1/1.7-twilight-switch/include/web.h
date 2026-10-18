#pragma once

#include <Arduino.h>

// Read-only Wi-Fi page and JSON. Nothing here waits for the network; the lamp doesn't depend on it.
namespace Web {
void begin();                // STA mode, hostname, starts connecting, returns at once
void handle(uint32_t nowMs); // call every loop: watches the connection, serves clients
}  // namespace Web
