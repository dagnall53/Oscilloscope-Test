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

bool Read_CTS (void);
void Set_CTS (bool set);


bool Data_RTS (void);
void Set_Data_RTS( bool set);
void ResetNumberofSamplesread(void);
int readNumberofsamplesRead(void);

void SetDigInputs(byte D0 , byte D1);

void scopeInit(void);

String scopeHandler(WebSocketsServer &WEBSOCKETOBJECT);

void ADCInit(void);
void setADCChannel(int CHANNEL);
int ADCRead(void);

void ADCHandler(void);
float ChannelRead1 (void);
float ChannelRead2 (void);
void SendHW_LIST(WebSocketsServer& WEBSOCKETOBJECT);

void ADCHandler(byte chan);
void ScalesInit(byte Data,byte Clock);
boolean ScalesConnected(void);
void SetScalesConnected(boolean set);
long readScales(byte Chan);
