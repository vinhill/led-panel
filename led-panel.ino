#include "secrets.h"
#include "definitions.h"
#include "weather.h"
#include "led_matrix.h"

#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

bool connect_wlan() {
  if (WiFi.isConnected())
    WiFi.disconnect();

  Serial.print("Connecting to '");
  Serial.print(WLAN_SSID);
  Serial.println("'");

  if (WiFi.begin(WLAN_SSID, WLAN_SSID) == WL_CONNECT_FAILED) {
    Serial.println("Connection failed");
    return false;
  }
  unsigned long start = millis();

  while(WiFi.status() != WL_CONNECTED && millis() - start < WLAN_CONNECT_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WLAN connection failed (timeout).");
    return false;
  } else {
    Serial.print("Connected to WLAN, IP: ");
    Serial.print(WiFi.localIP());
    return true;
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {
    //noop
  }

  if (!connect_wlan()) {
    Serial.println("Could not connect to WLAN, functionality limited.");
  }else {
    Serial.println("Setting up time provider via NTP");
    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP, NTP_SERVER, TIMEZONE_OFFSET, NTP_SYNC_INTERAL);
    timeClient.begin();
    timeClient.update();

    Serial.println("Setting up weather provider");
    WiFiClient client;
    Weather weather(&client);
    weather.sync();
    weather.log();
  }

  setup_led_matrix();
}

void loop() {
  random_pixels(5000);
  wolfram(5000);
  perlin_noise(5000);
  game_of_life(10000);
  wolfram(5000);
}
