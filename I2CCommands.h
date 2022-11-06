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

#include <WebSocketsServer.h>


void scanI2CAddressAndRegisters(WebSocketsServer &WEBSOCKETOBJECT);
void scanI2CAddress(WebSocketsServer &WEBSOCKETOBJECT);
void scanI2CRegisters(WebSocketsServer &WEBSOCKETOBJECT, byte ADDRESS);
boolean SinglescanI2CAddress(WebSocketsServer &WEBSOCKETOBJECT,byte address);
boolean SinglescanI2CAddress(byte address);
