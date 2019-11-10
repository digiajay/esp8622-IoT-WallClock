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
const size_t capacity = JSON_ARRAY_SIZE(3) + 2 * JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 480;
//const size_t capacity = JSON_OBJECT_SIZE(15) + 290;
DynamicJsonDocument doc(capacity);

#include <TimeLib.h>

/* For LCD Display*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* For WiFi connection and http API requests */
WiFiClient client;
HTTPClient http;

//Globals
long sys_sunset, sys_sunrise;
float main_temp_celcius, main_min_temp_celcius, main_max_temp_celcius = -273.15;
String week;

//Setup before starting the loop
void setup()
{
  lcd.init();
  lcd.backlight();

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(250);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin("vodafone-01D4", "XMLVU8SGGFHLRJ");
}

//Globals
int timeAPIcalls = 0; //For time api calls at every 120 seconds
int weatherAPIcalls = 0 ; //For weather api calls every 600
int displaySwitch=0; //For alternatively displaying time, temprature and sun timings


//Main loop which iterates all the time
void loop()
{
  //Call time API to sync time at every 1 minute.  Loop runs at 500ms
  if(timeAPIcalls%120 == 0)
    getTimeDate();
  //Call weather API to sync at every 2 minute.  Loop runs at 500ms
  if(weatherAPIcalls%240 == 0)
    getWeatherData();

  //switching to display the time and weather in interleave time. 
  switch (displaySwitch) {
  case 10: //Diplay temperature detils at every 5 seconds for 2 seconds
    if (main_max_temp_celcius > -270){
      displayTemperature();
      delay(2000);
    }
    else
      getWeatherData();
    break;
  case 20://Diplay sun timings at every 5 seconds for 2 seconds
    displaySun();
    displaySwitch = 0;
    delay(2000);
    break;
  default://By default in other free time display times.
    if (timeStatus() != timeNotSet)
      displayClock();
    else
      getTimeDate();
    break;
  }

  delay(500);
  
  timeAPIcalls++;
  weatherAPIcalls++;
  displaySwitch++;
}

//Display the clock in the LCD display.
void displayClock()
{
  String timenow = hour() + printDigits(minute()) + printDigits(second()) + "  " + week; //extra space to clear the other displayed chars
  String date = String(day()) + "/" + monthShortStr(month()) + "/" + year() + " " + dayShortStr(weekday()) + "       "; //extra space to clear the other displayed chars
  lcd.setCursor(0, 0);
  lcd.print(timenow);
  lcd.setCursor(0, 1);
  lcd.print(date);
  Serial.print("[Time] " + timenow + "\n" + date + "\n");
}

//Display the temperature details in the LCD display.
void displayTemperature()
{
  String Line1 = "Temp: " + String(main_temp_celcius) + "\"C           ";
  String Line2 = String(main_min_temp_celcius) + "\"C" + " to " + String(main_max_temp_celcius) +"\"C          "  ;
  lcd.setCursor(0, 0);
  lcd.print(Line1);
  lcd.setCursor(0, 1);
  lcd.print(Line2);
  Serial.print("[DisplayTemperature] " + Line1 + "\n" + Line2 + "\n");
}

//Display the sunrise and sunset time in the LCD display.
void displaySun()
{
  String sunrise = "SunRise: " + String(hour(sys_sunrise)) + printDigits(minute(sys_sunrise)) + "   ";
  String sunset  = "SunSet : " + String(hour(sys_sunset))  + printDigits(minute(sys_sunset))  + "   ";
  lcd.setCursor(0, 0);
  lcd.print(sunrise);
  lcd.setCursor(0, 1);
  lcd.print(sunset);
  Serial.print("[DisplaySunTime] " + sunrise + "\n" + sunset+ "\n");
  Serial.print("[DisplaySunTime Long] " + String(sys_sunrise) + "\n" + String(sys_sunset) + "\n");
}

//Helper function to convert the minutes seconds with zero prefix
String printDigits(int digits)
{
  // utility function for digital clock display: prints preceding colon and leading 0
  String digitStr = ":";
  if (digits < 10)
    digitStr += '0';
  digitStr += String(digits);
  return digitStr;
}

//Call API to get the current time from internet
void getTimeDate()
{
  String timeURL = "http://worldtimeapi.org/api/ip";
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED))
  {
    Serial.print("[HTTP] begin...\n");
    Serial.print("[Request]" + timeURL);

    if (http.begin(client, timeURL))
    { // HTTP

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
          String payload = http.getString();
          Serial.println(payload);
          deserializeJson(doc, payload);
          int week_number = doc["week_number"];
          const char *datetime = doc["datetime"];
          long unixtime = doc["unixtime"]; // 1573308387
          week = " Wk#" + String(week_number);
          setTime(unixtime);
        }
      }
      else
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    }
    else
    {
      Serial.printf("[HTTP] Unable to connect\n");
    }
  }
}

//Call API to get the current weather from internet
void getWeatherData()
{
  String OpenWeatherMapURL = "http://api.openweathermap.org/data/2.5/weather?q=Galway&APPID=7f204cf6049360470bba69ef0c217db1";
  if (http.begin(client, OpenWeatherMapURL))
  { // HTTP
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String payload = http.getString();
        Serial.println(payload);
        deserializeJson(doc, payload);

        JsonObject main = doc["main"];
        float main_temp = main["temp"];         // 282.85
        int main_pressure = main["pressure"];   // 1006
        int main_humidity = main["humidity"];   // 66
        float main_temp_min = main["temp_min"]; // 282.59
        float main_temp_max = main["temp_max"]; // 283.15

        JsonObject sys = doc["sys"];
        int sys_type = sys["type"];               // 1
        int sys_id = sys["id"];                   // 1569
        const char *sys_country = sys["country"]; // "IE"
        sys_sunrise = sys["sunrise"];             // 1573285744
        sys_sunset = sys["sunset"];               // 1573318259

        main_temp_celcius = main_temp - 273.15;
        main_min_temp_celcius = main_temp_min - 273.15;
        main_max_temp_celcius = main_temp_max - 273.15;
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.printf("[HTTP] Unable to connect\n");
  }
}
