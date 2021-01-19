/*
   Broadcast BTC Price using an ESP8266 board

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

//#define _7segments_74HC595
#define _7segments_TM1637

#ifdef _7segments_74HC595
//Pins on the 74HC595 shift reg chip - correspondence with Arduino digital pins
unsigned int ST_CP = 12;     // RCLK
unsigned int SH_CP = 14;     // SCLK
unsigned int DS    = 13;     // DIO

int hexDigitValue[] = {
    0xFC,    /* Segments to light for 0  */
    0x60,    /* Segments to light for 1  */
    0xDA,    /* Segments to light for 2  */
    0xF2,    /* Segments to light for 3  */
    0x66,    /* Segments to light for 4  */
    0xB6,    /* Segments to light for 5  */
    0xBE,    /* Segments to light for 6  */
    0xE0,    /* Segments to light for 7  */
    0xFE,    /* Segments to light for 8  */
    0xF6     /* Segments to light for 9  */
};

void setDigit(unsigned int row, unsigned int digit, boolean decimalPoint)
{
      unsigned int rowSelector;
      unsigned int data;

      rowSelector = bit(3-row)<<4;
      data =  ~  hexDigitValue[ digit & 0xF ] ;
      if(decimalPoint)
        data &= 0xFE;
        
      // First 8 data bits all the way into the second 74HC595 
      shiftOut(DS, SH_CP, LSBFIRST, data );

      // Now shift 4 row bits into the first 74HC595 and latch
      digitalWrite(ST_CP, LOW);
      shiftOut(DS, SH_CP, LSBFIRST, rowSelector );
      digitalWrite(ST_CP, HIGH);
      
}

void displayNumber(unsigned int number){
    for(unsigned int i=0; i<4; i++){
      setDigit(i, number % 10, false); // display righmost 4 bits (1 digit)
      number = number / 10;  // roll on to the next digit
      delay(1);
    } 
}
#endif 

#ifdef _7segments_TM1637
#include <TM1637Display.h>
 
const int CLK = 14; //Set the CLK pin connection to the display
const int DIO = 12; //Set the DIO pin connection to the display
 
int numCounter = 0;
 
TM1637Display display(CLK, DIO); //set up the 4-Digit Display.
#endif

void setup() {
#ifdef _7segments_74HC595
  // Set Arduino pins as outputs
  pinMode(ST_CP, OUTPUT);
  pinMode(SH_CP, OUTPUT);  
  pinMode(DS,    OUTPUT);
#endif

#ifdef _7segments_TM1637
  display.setBrightness(0x0a); //set the diplay to maximum brightness
#endif

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
    float price = http_root["bpi"]["USD"]["rate_float"].as<float>();
    char ssid[255];
    String password =  String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX) + String(random(0xffff), HEX);
    unsigned int sat = (int)(100000000/price);
    Serial.println(sat);
    
 #ifdef _7segments_74HC595
    displayNumber(sat);
 #endif

 #ifdef _7segments_TM1637
    display.showNumberDec(sat);
 #endif
    snprintf(ssid, 255, "1 Bitcoin = %.0f $", price);
    WiFi.softAP(ssid, password.c_str());
  } else {
    Serial.printf("Failed http request with error: %s\n", https.errorToString(httpResponseCode).c_str());
  }
  delay(1000 * DELAY);
}
