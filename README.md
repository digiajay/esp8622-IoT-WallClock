# ESP8622 IoT Wall Clock
This will be a simple project on Wemos D1 Mini for building an IoT Clock.

## The aim of this project is to build an IoT Wall Clock with following features.
- Local clock running on ESP8266 so that it displays clock in the absense of WiFi connectivity.
- Local clock synchronizes with internet on connection
- Display the clock and other information in sufficiently large & low power consuming display.  Right now chosen dot-matrix-LED display 32x8.  Preferrably like 7-segment LCD display which has backpanel reflection and has good visiblity from far off and very low power consumption.
- Run off on a battery
- Standard enclosure to look as good as end-product.  May be a 3D printed one if not found standard enclosures.
- Web server to get user configurations (like WiFi credentials, news feed preferences, weather preferences, tweet feeds etc..)
- OTA Update
- Scheduler to schedule the different functions and looks parallel activities.  Retrieving (or synchronizaing) time and weather data takes long time than usual and scheduler would help in keep running the clock.  https://www.arduino.cc/en/Tutorial/MultipleBlinks
- Maybe a simple piezzo buzzer to alarm.  Controllable from Webserver.  

V0.1 Video 

https://www.youtube.com/watch?v=pELyMVJ7KlI
