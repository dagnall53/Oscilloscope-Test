#include <Arduino.h>
#include <Wire.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include "I2CCommands.h"
#include "miniDB.h"

int ScreenUpdate(int A) ;
int Screen_U_time();

void SetDigInputs(byte D0 , byte D1);

void scopeInit(void);

String scopeHandler(WebSocketsServer &WEBSOCKETOBJECT);

void ADCInit(void);
void setADCChannel(int CHANNEL);
int ADCRead(void);
void ADCHandler(byte chan);
void ScalesInit(byte Data,byte Clock);
boolean ScalesConnected(void);
void SetScalesConnected(boolean set);
long readScales(byte Chan);

