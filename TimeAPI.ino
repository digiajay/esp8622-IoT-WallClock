/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

/*JSON Parser*/
#include <ArduinoJson.h>
const size_t capacity = JSON_OBJECT_SIZE(15) + 290;
DynamicJsonDocument doc(capacity);


/* For LCD Display*/
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  lcd.init();
  lcd.backlight();
  
  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin("vodafone-01D4", "XMLVU8SGGFHLRJ");

}

void loop() {
  // wait for WiFi connection
  String requestURL = "http://worldtimeapi.org/api/ip";

  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    Serial.print("[Request]" + requestURL);
    if (http.begin(client, requestURL)) {  // HTTP


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
          deserializeJson(doc, payload);
          int week_number = doc["week_number"];
          const char* datetime = doc["datetime"];
          String date = String(datetime).substring(0,10);
          String week = " Wk#" + String(week_number);
          String timenow = String(datetime).substring(11,16);
          lcd.setCursor(0,0);
          lcd.print(timenow);
          lcd.setCursor(0,1);
          lcd.print(date + week);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP] Unable to connect\n");
    }
  }

  delay(5000);
}
