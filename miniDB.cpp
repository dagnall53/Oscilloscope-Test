#include "miniDB.h"
#include "ScopeCommands.h"
int WS_Timer,sampleuSTimer;
bool dataLogFlag;
bool dataTAREFlag;
bool uartScopeFlag;
bool DuplexModeFlag;
bool _scopePause;
bool _OTA_ACTIVE;
bool _ACK,_HasBeenSent;
bool _RTS ;
String uartScopeData;
String adcScopeData1;
String adcScopeData2;
String channelMode1;  
String channelMode2;
int _baudRate;
int _MAX_Samples = 400;   //  testd to 300 (faults at 400?) with fast 1 ch 8266 mode..  1000 not found limit in slow modes. should be set to number across the screen! . 

long _MyTempTime,_MyTimer[]={0,0,0,0,0};
long _TTemp,           _T[]={0,0,0,0,0};
bool _set[5];
char _Loc[5][25];
// test functions
void _printStatus(char* location){
  Serial.printf("AT <%s> Pause<%i>  Data_RTS<%i>  HBS<%i>  ACK<%i>   ",location,PAUSE(), Data_RTS(),HBS(),  ACK() );
  Serial.println();
}



void _StartTestTimers(){
  
  
  for (int i=0 ;i<=4 ;i++){
    strcpy(_Loc[i], ""); 
    _set[i]=false;
    _MyTimer[i] = micros();
  }
  _MyTempTime = micros();
 }

void _Mark_Time(int input , char* location ){
  _T[input]= micros()-_MyTimer[input];
  strcpy(_Loc[input],location);
  _set[input]=true;
  
}

long _PrintTime( int input){
  return _T[input];
}

void _printalltimes(){
  
 Serial.printf(" Marked Times ");
 for (int i=0 ;i<=4 ;i++){
  if ( _set[i]){Serial.printf("(%s):%i ",_Loc[i],_PrintTime(i));}
 }Serial.println();
 }

bool HBS(void){
  return _HasBeenSent;
  }
  void SetHBS(bool set) {
 //if (set) {Serial.println( "SetHBS ON ");}else{Serial.println ( "SetHBS OFF ");}//Serial.println(" set HBS %s", set) 
// if ((set)) {Serial.print( "<");}
 _HasBeenSent = set;
 }


bool Data_RTS (void){
  return _RTS;
}
void Set_Data_RTS( bool set){
  // if false also reset Numberof samples read counter ?
  _RTS = set;
}


bool ACK(void){
  return _ACK;
  }

void _SetACK( bool set){
     _ACK=set; 
}


int MAX_Samples(){
  return _MAX_Samples;
}

void SetNSamples(int samples){
  _MAX_Samples = samples;
 // _printStatus("Updating Nsamples");
 Serial.print("N samples is now:");Serial.println(samples);
 
}


// terminal variables

void SetBaud (int input){
  _baudRate= input;
  Serial.print("Baud set to:");Serial.println(_baudRate);
}
 int BaudRate(){
  return _baudRate;  
 }

void DoingOTA( bool input){
 _OTA_ACTIVE = input;
 }
bool OTA_ON(){
  return _OTA_ACTIVE;
}

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



void setWS_Timer(int TIME)
{
  String  MSGdata ;
	WS_Timer = TIME ; 
  Serial.print(" Setting Websockets update Timer to:");
  Serial.print(WS_Timer);
  Serial.println("ms");

 //  MSGdata = "SCOPE WS_Timer ";  // if the html decided what the mtimer, i, I think we do not need to send it a copy back 
 //  MSGdata += String ( getWS_Timer(),DEC);    //Samples per second are 1000/ WS_Timer.
 //   BROADCAST(MSGdata);
 //   Serial.print("...updated WS_Timer , sending:");
 //   Serial.println(MSGdata);
}
int getWS_Timer(void)   // now is the update rate for the Websock send  
{
  if (WS_Timer > 1000) {return 1000;}  // limit update slowness to one update per 10 second
	return WS_Timer;  // ten samples updates per screen width (perhaps update later to account for multiplier)
}

/////////////////
//DATA LOG FLAG
/////////////////
void setDataLog(bool LOG)
{
  Serial.println("");
  Serial.print(" Setting data logger to:");
  Serial.print(LOG);
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


void setPAUSE(bool Pause){
  Serial.print(" Setting Pause to:");
  Serial.print(Pause);
  _scopePause = Pause;
  //_printStatus(" Setting_Pause ");
  if(!Pause){ Set_Data_RTS(false); _SetACK(true); // should trigger new samples. 
  }
}
bool PAUSE(){
  return _scopePause;
}


/////////////////
//ADC SCOPE DATA
/////////////////
void BuildScopeDataString(String DATA1, String DATA2,String DATA3){
  adcScopeData1 = adcScopeData1 + " "+DATA1+" "+DATA2+" "+ DATA3;
}



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
  Serial.print("Channel 1 :");Serial.println(MODE);
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
  Serial.print("Channel 2 :");Serial.println(MODE);
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

bool every(float number, int num2){
  return ((number/num2) == int(number/num2));
}
