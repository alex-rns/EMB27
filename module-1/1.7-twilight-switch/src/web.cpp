#include "web.h"
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include "config.h"
#include "event_log.h"
#include "lamp.h"
#include "light_sensor.h"
#include "motion.h"
#include "page.h"

// Credentials live in secrets.h (git ignores it). Without it the example file keeps the build working.
#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#define USING_EXAMPLE_SECRETS
#endif

namespace {
WebServer server(80);
bool serverRunning = false;
bool wasConnected = false;
uint32_t lastCheckMs = 0;

// Event texts are our own fixed strings without quotes or backslashes, so they need no JSON escaping.
// Returns false when the 1024-byte buffer is too small.
bool buildStateJson(char* out, size_t size, uint32_t nowMs) {
    const uint32_t since = Motion::msSinceMotion(nowMs);
    char motionAgo[12];
    if (since == Motion::NEVER) {
        snprintf(motionAgo, sizeof motionAgo, "null");
    } else {
        snprintf(motionAgo, sizeof motionAgo, "%lu", static_cast<unsigned long>(since));
    }

    int n = snprintf(out, size,
                     "{\"uptimeMs\":%lu,\"raw\":%u,\"dark\":%s,\"darkBelow\":%u,\"lightAbove\":%u,"
                     "\"motionAgoMs\":%s,\"lamp\":%s,\"leftS\":%lu,\"events\":[",
                     static_cast<unsigned long>(nowMs), static_cast<unsigned>(LightSensor::raw()),
                     LightSensor::isDark() ? "true" : "false", static_cast<unsigned>(Config::THRESHOLD_DARK),
                     static_cast<unsigned>(Config::THRESHOLD_LIGHT), motionAgo, Lamp::isOn() ? "true" : "false",
                     static_cast<unsigned long>(Lamp::secondsLeft(nowMs)));
    if (n < 0 || static_cast<size_t>(n) >= size) return false;
    size_t len = static_cast<size_t>(n);

    for (size_t i = 0; i < EventLog::count(); i++) {
        const Event& e = EventLog::get(i);
        n = snprintf(out + len, size - len, "%s{\"atMs\":%lu,\"text\":\"%s\"}", i == 0 ? "" : ",",
                     static_cast<unsigned long>(e.atMs), e.text);
        if (n < 0 || static_cast<size_t>(n) >= size - len) return false;
        len += static_cast<size_t>(n);
    }

    n = snprintf(out + len, size - len, "]}");
    return n >= 0 && static_cast<size_t>(n) < size - len;
}

void handleRoot() { server.send_P(200, "text/html", PAGE); }

void handleState() {
    char json[1024];
    if (!buildStateJson(json, sizeof json, millis())) {
        server.send(500, "text/plain", "json overflow");
        return;
    }
    server.send(200, "application/json", json);
}

void handleNotFound() { server.send(404, "text/plain", "not found"); }

void onConnected(uint32_t nowMs) {
    char text[sizeof(Event::text)];
    snprintf(text, sizeof text, "Wi-Fi connected, IP %s", WiFi.localIP().toString().c_str());
    EventLog::add(nowMs, text);

    if (!serverRunning) {  // first connect only
        MDNS.begin(Config::HOSTNAME);
        MDNS.addService("http", "tcp", 80);
        server.begin();
        serverRunning = true;
    }
}
}  // namespace

namespace Web {
void begin() {
#ifdef USING_EXAMPLE_SECRETS
    Serial.println("Wi-Fi: no secrets.h, using secrets.example.h");
#endif
    // 2.0.17: setHostname() only stores the name; mode() applies it to the STA interface, so it goes first.
    WiFi.setHostname(Config::HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // returns at once; the connection happens in the background

    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/state", HTTP_GET, handleState);
    server.onNotFound(handleNotFound);
}

void handle(uint32_t nowMs) {
    if (nowMs - lastCheckMs >= Config::WIFI_CHECK_MS) {
        lastCheckMs = nowMs;
        const bool connected = WiFi.status() == WL_CONNECTED;
        if (connected && !wasConnected) {
            onConnected(nowMs);
        } else if (!connected && wasConnected) {
            EventLog::add(nowMs, "Wi-Fi lost");
        }
        if (!connected) WiFi.reconnect();
        wasConnected = connected;
    }
    if (serverRunning) server.handleClient();
}
}  // namespace Web
