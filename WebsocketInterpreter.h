#include <Arduino.h>
#include <Wire.h>
// #ifdef ESP32  
//   #include <WiFi.h>
//  #else
//   #include <ESP8266WiFi.h>
//   #include <ESP8266WiFiMulti.h>
//   #include <Hash.h>
//   #include <ESP8266WebServer.h>
//   #include <ESP8266mDNS.h>
// #endif

#include "ScopeCommands.h"
#include "miniDB.h"



void webSocketDataInterpreter(WebSocketsServer &WEBSOCKETOBJECT, String WEBSOCKETDATA);
