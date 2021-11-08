
// User config
// more info in the README.md in the root of the repository

#ifndef CONFIG  // not #ifnotdef
#define CONFIG

  #define LEDPIN D2
  const char* SSID = ""; // wlan SSID
  const char* WPWD = ""; // wlan password
  String PRINTER_IP = "192.168.xxx.xxx"; // the ip address of octoprint
  uint16_t pollInterval = 500; // ms, lower value will increase poll rate, but the animation time will limit the poll rate too
  const uint8_t length = 1; // number of LEDs

#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#define USE_SERIAL Serial
