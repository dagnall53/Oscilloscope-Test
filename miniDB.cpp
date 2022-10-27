#include "miniDB.h"

int msTimer,sampleuSTimer;
bool dataLogFlag;
bool dataTAREFlag;
bool uartScopeFlag;
bool DuplexModeFlag;
String uartScopeData;
String adcScopeData1;
String adcScopeData2;
String channelMode1;
String channelMode2;

/////////////////
//TIMER VARIABLE
/////////////////
void setsampleuSTimer (int TIME){
  sampleuSTimer = TIME;
  Serial.print(" Setting sample timer to:");
  Serial.print(sampleuSTimer);
  Serial.println(" uS");
}
int getsampleuSTimer (void){
  return sampleuSTimer;
}



void setMsTimer(int TIME)
{
  String  MSGdata ;
	msTimer = TIME ; 
  Serial.print(" Setting Websockets update Timer to:");
  Serial.print(msTimer);
  Serial.println("ms");

 //  MSGdata = "SCOPE MSTIMER ";  // if the html decided what the mtimer, i, I think we do not need to send it a copy back 
 //  MSGdata += String ( getMsTimer(),DEC);    //Samples per second are 1000/ mstimer.
 //   BROADCAST(MSGdata);
 //   Serial.print("...updated msTimer , sending:");
 //   Serial.println(MSGdata);
}
int getMsTimer(void)   // now is the update rate for the Websock send  
{
  if (msTimer > 1000) {return 1000;}  // limit update slowness to one update per 10 second
	return msTimer;  // ten samples updates per screen width (perhaps update later to account for multiplier)
}

/////////////////
//DATA LOG FLAG
/////////////////
void setDataLog(bool LOG)
{
  dataLogFlag = LOG;
}
bool getDataLog(void)
{
  return dataLogFlag;
}
/////////////
//Scales Flags
//////////////
void setTARE(bool LOG)
{
  dataTAREFlag = LOG;
}
bool getTARE(void)
{
  return dataTAREFlag;
}

/////////////////
//UART SCOPE DATA
/////////////////
void setUartScopeFlag(bool UARTSCOPEFLAG)
{
  uartScopeFlag = UARTSCOPEFLAG;
}
bool getUartScopeFlag(void)
{
  return uartScopeFlag;
}
void setDuplexMode( bool Duplex){
 if (Duplex) {Serial.println(" Setting Duplex Mode");}
 else {Serial.println(" Clearing Duplex Mode");}
  DuplexModeFlag = Duplex;
}
bool getDuplexMode (void)
{
  return DuplexModeFlag;
}


void setUartScopeData(String UARTSCOPEDATA)
{
  uartScopeData += " ";
  uartScopeData += UARTSCOPEDATA;
}
String getUartScopeData(void)
{
  return uartScopeData;
}
void clearUartScopeData(void)
{
  uartScopeData = "";
}

/////////////////
//ADC SCOPE DATA
/////////////////
void addADCScopeData1(String ADCSCOPEDATA)
{
  adcScopeData1 += " ";
  adcScopeData1 += ADCSCOPEDATA;
}

String getADCScopeData1(void)
{
  return adcScopeData1;
}
void addADCScopeData2(String ADCSCOPEDATA)
{
  adcScopeData2 += " ";
  adcScopeData2 += ADCSCOPEDATA;
}
String getADCScopeData2(void)
{
  return adcScopeData2;
}
void clearADCScopeData1(void)
{
  adcScopeData1 = "";
}
void clearADCScopeData2(void)
{
  adcScopeData2 = "";
}

/////////////////
//SCOPE PARAMETERS
/////////////////




void setChannelMode1(String MODE)
{
  channelMode1 = "";
  channelMode1 = MODE;
  if(channelMode1 == "UART")
  {
    setUartScopeFlag(true);    
  }
  else
  {
    setUartScopeFlag(false);
  }
}
void setChannelMode2(String MODE)
{
  channelMode2 = "";
  channelMode2 = MODE;
  if(channelMode2 == "UART")
  {
    setUartScopeFlag(true);    
  }
  else
  {
    setUartScopeFlag(false);
  }
}
String getChanneMode1(void)
{
  return channelMode1;
}
String getChanneMode2(void)
{
  return channelMode2;
}
