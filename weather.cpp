#include "weather.h"
#include "secrets.h"
#include "definitions.h"

#include <HTTPClient.h>
#include <time.h>

Weather::Weather(WiFiClient* _client) {
    client = _client;
}

void Weather::sync() {
    const char server[] = "https://api.tomorrow.io/v4/timelines";
    
    DynamicJsonDocument req(2048);
    req["location"] = GPS_COORDINATES;
    req["units"] = "metric";
    req["api_key"] = TOMORROW_API_KEY;

    JsonArray timesteps = req.createNestedArray("timesteps");
    timesteps.add("1h");
    timesteps.add("1d");

    JsonArray fields = req.createNestedArray("fields");
    fields.add("temperature");
    fields.add("humidity");
    fields.add("windSpeed");
    fields.add("rainIntensity");
    fields.add("precipitationProbability");
    fields.add("precipitationType");
    fields.add("sunriseTime");
    fields.add("sunsetTime");
    fields.add("cloudCover");
    fields.add("uvHealthConcern");
    fields.add("weatherCodeFullDay");
    fields.add("weatherCodeDay");
    fields.add("weatherCodeNight");
    fields.add("epaIndex");

    HTTPClient http;
    http.begin(*client, server);
    String json;
    serializeJson(req, json);
    http.POST(json);

    res = http.getString();

    time(&last_update);
}

void Weather::log() {
    Serial.print("Weather: ");
    Serial.println(res);
}

bool Weather::should_sync() {
    time_t now;
    time(&now);
    return (now - last_update) > WEATHER_SYNC_INTERVAL;
}