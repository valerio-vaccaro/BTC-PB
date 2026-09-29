/*
   Broadcast BTC Price

   Based on:
   - WiFiManager
   - https://blockchain.info/ticker API
*/


#include <ArduinoJson.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <TM1638.h>
#include <TM16xxDisplay.h>
#include <TM16xxButtons.h>
#endif

#include <WiFiManager.h>

#define AGENT "BTC_Price_Broadcaster/0.0.1"
#define API_URL "https://blockchain.info/ticker"
#define DELAY 60

#define MAX_CURRENCIES 30
struct CurrencyPrice {
    String symbol;
    float price;
};
CurrencyPrice currencyPrices[MAX_CURRENCIES];
int numCurrencies = 0;
int cur_counter = 9; // EUR

#ifdef ESP32
TM1638 module(25, 26, 27);
TM16xxButtons buttons(&module);  
TM16xxDisplay display(&module, 8);

u8_t status = 0;
String ticker = "USD";
String symbol = "USd";
float price = 1;
unsigned int sat;

static unsigned long updateTime = millis();
static unsigned long ulTime = millis() - 1000000;

#define MAX_PAGES 2
static bool page = 0;

void fnClick(byte nButton) {
  Serial.print(F("Button "));
  Serial.print(nButton);
  Serial.println(F(" click."));
  // 0 ARS - Argentine Peso
  // 1 AUD - Australian Dollar
  // 2 BRL - Brazilian Real
  // 3 CAD - Canadian Dollar - 6
  // 4 CHF - Swiss Franc - 5
  // 5 CLP - Chilean Peso
  // 6 CNY - Chinese Yuan - 3
  // 7 CZK - Czech Koruna 
  // 8 DKK - Danish Krone
  // 9 EUR - Euro - 0
  // 10 GBP - British Pound - 2
  // 11 HKD - Hong Kong Dollar
  // 12 HRK - Croatian Kuna
  // 13 HUF - Hungarian Forint 
  // 14 INR - Indian Rupee
  // 15 ISK - Icelandic Króna
  // 16 JPY - Japanese Yen - 4
  // 17 KRW - South Korean Won
  // 18 NGN - Nigerian Naira
  // 19 NZD - New Zealand Dollar
  // 20 PLN - Polish Zloty
  // 21 RON - Romanian Leu
  // 22 RUB - Russian Ruble - 7 
  // 23 SEK - Swedish Krona
  // 24 SGD - Singapore Dollar
  // 25 THB - Thai Baht
  // 26 TRY - Turkish Lira
  // 27 TWD - New Taiwan Dollar
  // 28 USD - US Dollar - 1
  switch (nButton){
    case 16:
      cur_counter = 9 ; // EUR
      module.setLED(TM1638_COLOR_RED, 0);
      module.setLED(TM1638_COLOR_GREEN, 1);
      module.setLED(TM1638_COLOR_GREEN, 2);
      module.setLED(TM1638_COLOR_GREEN, 3);
      module.setLED(TM1638_COLOR_GREEN, 4);
      module.setLED(TM1638_COLOR_GREEN, 5);
      module.setLED(TM1638_COLOR_GREEN, 6);
      module.setLED(TM1638_COLOR_GREEN, 7);
      break;
    case 18:
      cur_counter = 28; // USD
      module.setLED(TM1638_COLOR_GREEN, 0);
      module.setLED(TM1638_COLOR_RED, 1);
      module.setLED(TM1638_COLOR_GREEN, 2);
      module.setLED(TM1638_COLOR_GREEN, 3);
      module.setLED(TM1638_COLOR_GREEN, 4);
      module.setLED(TM1638_COLOR_GREEN, 5);
      module.setLED(TM1638_COLOR_GREEN, 6);
      module.setLED(TM1638_COLOR_GREEN, 7);
      break;
    case 20:
      cur_counter = 10 ; // GBP
      module.setLED(TM1638_COLOR_GREEN, 0);
      module.setLED(TM1638_COLOR_GREEN, 1);
      module.setLED(TM1638_COLOR_RED, 2);
      module.setLED(TM1638_COLOR_GREEN, 3);
      module.setLED(TM1638_COLOR_GREEN, 4);
      module.setLED(TM1638_COLOR_GREEN, 5);
      module.setLED(TM1638_COLOR_GREEN, 6);
      module.setLED(TM1638_COLOR_GREEN, 7);
      break;
    case 22:
      cur_counter = 6; // CNY
      module.setLED(TM1638_COLOR_GREEN, 0);
      module.setLED(TM1638_COLOR_GREEN, 1);
      module.setLED(TM1638_COLOR_GREEN, 2);
      module.setLED(TM1638_COLOR_RED, 3);
      module.setLED(TM1638_COLOR_GREEN, 4);
      module.setLED(TM1638_COLOR_GREEN, 5);
      module.setLED(TM1638_COLOR_GREEN, 6);
      module.setLED(TM1638_COLOR_GREEN, 7);
      break;
    case 17:
      cur_counter = 16; // JPY
      module.setLED(TM1638_COLOR_GREEN, 0);
      module.setLED(TM1638_COLOR_GREEN, 1);
      module.setLED(TM1638_COLOR_GREEN, 2);
      module.setLED(TM1638_COLOR_GREEN, 3);
      module.setLED(TM1638_COLOR_RED, 4);
      module.setLED(TM1638_COLOR_GREEN, 5);
      module.setLED(TM1638_COLOR_GREEN, 6);
      module.setLED(TM1638_COLOR_GREEN, 7);
      break;
    case 19:
      cur_counter = 4; // CHF
      module.setLED(TM1638_COLOR_GREEN, 0);
      module.setLED(TM1638_COLOR_GREEN, 1);
      module.setLED(TM1638_COLOR_GREEN, 2);
      module.setLED(TM1638_COLOR_GREEN, 3);
      module.setLED(TM1638_COLOR_GREEN, 4);
      module.setLED(TM1638_COLOR_RED, 5);
      module.setLED(TM1638_COLOR_GREEN, 6);
      module.setLED(TM1638_COLOR_GREEN, 7);
      break;
    case 21:
      cur_counter = 3; // CAD
      module.setLED(TM1638_COLOR_GREEN, 0);
      module.setLED(TM1638_COLOR_GREEN, 1);
      module.setLED(TM1638_COLOR_GREEN, 2);
      module.setLED(TM1638_COLOR_GREEN, 3);
      module.setLED(TM1638_COLOR_GREEN, 4);
      module.setLED(TM1638_COLOR_GREEN, 5);
      module.setLED(TM1638_COLOR_RED, 6);
      module.setLED(TM1638_COLOR_GREEN, 7);
      break;
    case 23:
      cur_counter = 22; // RUB
      module.setLED(TM1638_COLOR_GREEN, 0);
      module.setLED(TM1638_COLOR_GREEN, 1);
      module.setLED(TM1638_COLOR_GREEN, 2);
      module.setLED(TM1638_COLOR_GREEN, 3);
      module.setLED(TM1638_COLOR_GREEN, 4);
      module.setLED(TM1638_COLOR_GREEN, 5);
      module.setLED(TM1638_COLOR_GREEN, 6);
      module.setLED(TM1638_COLOR_RED, 7);
      break;
    default:
      break;
  }
}
#endif

void setup() {
  Serial.begin(115200);
  Serial.println(AGENT);
#ifdef ESP32
  buttons.attachClick(fnClick);
  display.setIntensity(4); // Max 7
  display.println("satoshi");

  module.setLED(TM1638_COLOR_RED, 0);
  module.setLED(TM1638_COLOR_GREEN, 1);
  module.setLED(TM1638_COLOR_GREEN, 2);
  module.setLED(TM1638_COLOR_GREEN, 3);
  module.setLED(TM1638_COLOR_GREEN, 4);
  module.setLED(TM1638_COLOR_GREEN, 5);
  module.setLED(TM1638_COLOR_GREEN, 6);
  module.setLED(TM1638_COLOR_GREEN, 7);
#endif
  WiFiManager wifiManager;
  String password =  "BTCBroadcaster";
  Serial.printf("Starting a temporary AP with ssid BTC_Price_Broadcaster and password %s\n", password.c_str());
  if (!wifiManager.autoConnect("BTC_Price_Broadcaster", password.c_str())) {
    Serial.println(F("failed to connect, we should reset as see if it connects"));
    delay(3000);
    ESP.restart();
    delay(5000);
  }
  Serial.println("connected!");
  Serial.print("local ip ");
  Serial.println(WiFi.localIP());
  WiFi.mode(WIFI_AP_STA);
}

void loop() {
  uint32_t dwButtons = buttons.tick();
  if(millis() - updateTime > 1000) {
    updateTime = millis();

    // Create a new AP for each currency
    char ssid[255];
    String password =  String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX);
    price = currencyPrices[cur_counter].price;
    
    snprintf(ssid, 255, "1 Bitcoin = %.0f %s", price, currencyPrices[cur_counter].symbol.c_str());
    WiFi.softAP(ssid, password.c_str());
    Serial.println(ssid);

    sat = (int)(100000000/price);
    switch(page){
    case 0:
      #ifdef ESP32
      display.println(currencyPrices[cur_counter].symbol.c_str());
      #endif
      break;
    case 1:
      #ifdef ESP32
      display.print(sat);
      display.println(" sat");
      #endif
      break;
    default:
      break;
    }
    page = (page + 1) % MAX_PAGES;
  }

  if(millis() - ulTime > 1000 * DELAY) {
    ulTime = millis();
  #ifdef ESP8266
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient https;
    https.addHeader("User-Agent", AGENT);
    https.addHeader("Accept", "*/*");
    https.begin(*client, API_URL);
    int httpResponseCode = https.GET();
    if (httpResponseCode > 0) {
      String response = https.getString();
      Serial.println(response);
      StaticJsonDocument<1000> http_root;
      DeserializationError http_error = deserializeJson(http_root, response);
      if (http_error) {
        Serial.printf("Failed to read json from %s with error %d\n", API_URL, http_error);
      }
      price = http_root["USD"]["last"].as<float>();
    } else {
      Serial.printf("Failed http request with error: %s\n", https.errorToString(httpResponseCode).c_str());
    }
    char ssid[255];
    String password =  String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX);
    sat = (int)(100000000/price);
    Serial.println(sat);
    snprintf(ssid, 255, "1 Bitcoin = %.0f $", price);
    WiFi.softAP(ssid, password.c_str());
  #endif
  #ifdef ESP32
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient https;
      https.addHeader("User-Agent", AGENT);
      https.addHeader("Accept", "*/*");
      try {
        https.begin(API_URL);
        int httpCode = https.GET();
        if (httpCode == HTTP_CODE_OK) {
            String response = https.getString();
            Serial.println(response);
            StaticJsonDocument<1000> http_root;
            DeserializationError http_error = deserializeJson(http_root, response);

            if (http_error) {
              Serial.printf("Failed to read json from %s with error %d\n", API_URL, http_error);
            }
            
            // Store all currency prices
            numCurrencies = 0;
            for (JsonPair kv : http_root.as<JsonObject>()) {
                if (numCurrencies < MAX_CURRENCIES) {
                    currencyPrices[numCurrencies].symbol = kv.key().c_str();
                    currencyPrices[numCurrencies].price = kv.value()["last"].as<float>();
                    numCurrencies++;
                }
            }
        }        
        https.end();
      } catch(...) {
        https.end();
      }
    }
  #endif

  }

}