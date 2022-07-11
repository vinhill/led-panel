#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

class Weather {
public:
    Weather(WiFiClient *client);
    void sync();
    void log();
    bool should_sync();

private:
    time_t last_update;
    WiFiClient* client;
    String res;

    String unix_to_iso8601(time_t unix_time);
};