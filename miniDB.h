#include <Arduino.h>


void _printStatus(char* location);
void SETESP32ANALOG(int pin1, int pin2 );// (PIN1, atten, pin2,atten)
int ANALOGREAD1( ); //esp32 and 8266 compatible. 
int ANALOGREAD2( ); //esp32 and 8266 compatible. 

// timing for tests
void _StartTestTimers();
void _Mark_Time(int input , char* location );
long _PrintTime( int input);
void _printalltimes();  // prints out all marked times

bool ACK(void);
void _SetACK( bool set);

bool HBS(void);  // prevents double trigger of Websoc send 
void SetHBS(bool set);


bool Data_RTS (void);
void Set_Data_RTS( bool set);

// Terminal 
void SetBaud (int input);
 int BaudRate();

int MAX_Samples();
void SetNSamples(int samples); // Updating N samples for size of the batch 

void DoingOTA( bool input);
bool OTA_ON();


//TIMER VARIABLE
void BROADCAST(String MSG); 
void setWS_Timer(int TIME);
int getWS_Timer(void);

void setsampleuSTimer (int PERIOD);
int getsampleuSTimer (void);

//DATA LOG FLAG
void setDataLog(bool LOG);
bool getDataLog(void);
//SCALES TARE FLAG
void setTARE(bool LOG);
bool getTARE(void);
//UART SCOPE DATA
void setUartScopeFlag(bool UARTSCOPEFLAG);
bool getUartScopeFlag(void);
void setUartScopeData(String UARTSCOPEDATA);
String getUartScopeData(void);
void clearUartScopeData(void);

//ADC SCOPE DATA
void BuildScopeDataString(String DATA1, String DATA2,String DATA3);


void addADCScopeData(String ADCSCOPEDATA);
String getADCScopeData(void);
void setLastADCScopeData(String LASTADCSCOPEDATA);
String getLastADCScopeData(void);
void clearADCScopeData(void); //DAG
void addADCScopeData1(String ADCSCOPEDATA);
void addADCScopeData2(String ADCSCOPEDATA);
String getADCScopeData1(void);
String getADCScopeData2(void);
void clearADCScopeData1(void);
void clearADCScopeData2(void);

bool every(float number, int num2);
//SCOPE PARAMETERS
void setPAUSE(bool Pause);
bool PAUSE();


void setDuplexMode( bool Duplex);
bool getDuplexMode (void);
void setChannelMode1(String MODE);
void setChannelMode2(String MODE);
String getChanneMode1(void);
String getChanneMode2(void);
