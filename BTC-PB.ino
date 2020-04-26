/*
   Boadcast BTC Price using an ESP8266 board

   Based on:
   - WiFiManager
   - https://api.coindesk.com/v1/bpi/currentprice.json API
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define AGENT "BTC_Price_Broadcaster/0.0.1"
#define API_URL "https://api.coindesk.com/v1/bpi/currentprice.json"
#define API_FINGERPRINT "F9 C6 A4 85 AE 53 AD 27 80 FC 75 D9 6C 1D 38 29 8C 32 7F BC"
#define DELAY 60

void setup() {
  Serial.begin(115200);
  Serial.println(AGENT);
  WiFiManager wifiManager;
  String password =  String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX);
  Serial.printf("Starting a temporary AP with ssid BTC_Price_Broadcaster and password %s\n", password.c_str());
  if (!wifiManager.autoConnect("BTC_Price_Broadcaster", password.c_str())) {
    Serial.println(F("failed to connect, we should reset as see if it connects"));
    delay(3000);
    ESP.reset();
    delay(5000);
  }
  Serial.println("connected!");
  Serial.print("local ip ");
  Serial.println(WiFi.localIP());
  WiFi.mode(WIFI_AP_STA);
}

void loop() {
  HTTPClient https;
  https.addHeader("User-Agent", AGENT);
  https.addHeader("Accept", "*/*");
  https.begin(API_URL, API_FINGERPRINT);
  int httpResponseCode = https.GET();
  if (httpResponseCode > 0) {
    String response = https.getString();
    Serial.println(response);
    StaticJsonDocument<1000> http_root;
    DeserializationError http_error = deserializeJson(http_root, response);
    if (http_error) {
      Serial.printf("Failed to read json from %s with error %d\n", API_URL, http_error);
    }
    float price = http_root["bpi"]["USD"]["rate_float"].as<float>();
    char ssid[255];
    String password =  String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX);
    snprintf(ssid, 255, "1 Bitcoin = %.0f $", price);
    WiFi.softAP(ssid, password.c_str());
  } else {
    Serial.printf("Failed http request with error: %s\n", https.errorToString(httpResponseCode).c_str());
  }
  delay(1000 * DELAY);
}
