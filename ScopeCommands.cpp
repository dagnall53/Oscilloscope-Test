#include "ScopeCommands.h"
#include <HX711.h>

HX711 scale;
const int _BufferSize = 512;
 int buffer[3][_BufferSize];  // limits at about 350 on the sends but try to have bigger for trigger capture

boolean ScalesPresent;
String channelModeOutput1;
String channelModeOutput2;
bool toggledChannelOffFlag1;
bool toggledChannelOffFlag2;
byte ADCAddress = 54;  //adc address on 12c

float CH1Scale = 1;  //DAG new variables for scaling everything
float CH2Scale = 1;
float CH1scaleset = -2.09;
float CH2scaleset = -2.09;
unsigned long sendTime = 0;
long TAREA = 0;
long TAREB = 0;
byte LastChanRead = 0;


int NumberofSamplesRead = 0;
float TestTriangle = 0;
float TestTriangle1 =0;
bool _RTS ;
bool _HasBeenSent;

byte ScopeDigInput0, ScopeDigInput1;
int Screen_update_time;
int ScreenUpdate(int A) {
  Screen_update_time = A;
}
int Screen_U_time() {
  return Screen_update_time;
}

bool HasBeenSent(void){
  
  return _HasBeenSent;
}
void SetHBS(bool set) {
 if (set) {Serial.println( "SetHBS ON ");}else{Serial.println ( "SetHBS OFF ");}//Serial.println(" set HBS %s", set) 
 if ((set)) {Serial.print( "<");}
 _HasBeenSent = set;
}


bool Data_RTS (void){
  return _RTS;
}
void Set_Data_RTS( bool set){
  // if false also reset Numberof samples read counter ?
  _RTS = set;
}

void SendHW_LIST(WebSocketsServer& WEBSOCKETOBJECT){
        channelModeOutput1 = "Hardware_LIST_(scales,50,60) ";
        channelModeOutput1 += String(ScalesConnected());  channelModeOutput1 += " " ;
        channelModeOutput1 += String(SinglescanI2CAddress(50)); channelModeOutput1 += " " ;
        channelModeOutput1 += String(SinglescanI2CAddress(60));
        WEBSOCKETOBJECT.broadcastTXT(channelModeOutput1);
}

void ResetNumberofSamplesread(){
NumberofSamplesRead =0;
}

int readNumberofsamplesRead(void){
  return NumberofSamplesRead;
}

void SetDigInputs(byte D0, byte D1) {
  ScopeDigInput0 = D0;
  ScopeDigInput1 = D1;
  pinMode(D0, INPUT_PULLUP);
  pinMode(D1, INPUT_PULLUP);
}

boolean ScalesConnected(void) {
  return ScalesPresent;
}
void ScalesInit(byte Data, byte Clock) {
  long reading = 0;
  LastChanRead = 100;
  pinMode(Data, INPUT_PULLUP);
  Serial.print("Testing for Scales...");
  scale.begin(Data, Clock);
  if (scale.is_ready()) {
    Serial.println("Ready");
    scale.set_gain(128);
    Serial.println("  HX711 PRESENT ");
    SetScalesConnected(1);  // selects A
  } else {
    Serial.println("NOT present");
    Serial.println(" No HX711 Present");
    SetScalesConnected(0);
  }

  // reading = scale.read();
  // Serial.println(reading);
  // if (reading != -1) {

  // } else {
  //   Serial.println(" No HX711 Present");
  //   SetScalesConnected(0);
  // }

  if (ScalesConnected()) {
    TAREA = 0;
    TAREB = 0;
    Serial.println("----------------------------");
    Serial.println("Initializing the A scale");
    Serial.print("Initial A and TARE_A: ");
    TAREA = scale.read_average(10);
    Serial.println(TAREA);
    Serial.println("----------------------------");
    scale.set_gain(32);  // select ch b
    Serial.println("Initializing the B scale");
    Serial.print("Initial B and TARE_B ");
    TAREB = scale.read_average(10);
    ;  // print a raw reading of chA with tare = 0
    Serial.println(TAREB);
  }
}
void SetScalesConnected(boolean set) {
  ScalesPresent = set;
}

long readScales(byte Chan) {
  long Reading;
  int average = 1;
  Reading = 0;
  if (ScalesConnected()) {
    if (Chan == 0) {  //CH A

      if (Chan != LastChanRead) { Serial.print("Setgain 0");
        scale.set_gain(128);
        Reading = scale.read();//dummy to get ready because this needs setting up
      }  
      //CH1scaleset);  this value is obtained by calibrating the scale with known weights; see the README for details
      LastChanRead = Chan;
      //Reading = (((scale.read() - TAREA) / CH1scaleset));
      Reading = (((scale.read_average(average) - TAREA) / CH1scaleset));
    }
    if (Chan == 1) {  // CH B
      if (Chan != LastChanRead) { Serial.print("Setgain 1");
        scale.set_gain(32);
        Reading = scale.read();//dummy to get ready
      }  
         //CH2scaleset);  This value is obtained by calibrating the scale with known weights; see the README for details
      LastChanRead = Chan;
      //Reading = (((scale.read() - TAREB) / CH2scaleset));
      Reading = (((scale.read_average(average) - TAREB) / CH2scaleset));
    }
  }
  return Reading;
}
void scopeInit(void) {

  setChannelMode1("");  // nb this does not set the initial settings, use "SELECTED" IN THE WEBSITE HTML CODES
  setChannelMode2("");
  toggledChannelOffFlag1 = false;
  toggledChannelOffFlag2 = false;
  channelModeOutput1 = "";
  channelModeOutput2 = "";
  // changed and webserver to defauult to var yPlotScaleFactor = 10;
  setDuplexMode(false);
  setUartScopeData("0");
  ADCInit();
}

String scopeHandler(WebSocketsServer& WEBSOCKETOBJECT) {
  String _output_summary = "";
// only doing "duplex mode" now.. 
        //Serial.print("duplex send");
        channelModeOutput1 = "SCOPE ADC DUPLEX";
        channelModeOutput1 += String(getADCScopeData1());  // getADCScopeData1 has special DUPLEX mode that captures BOTH channels 
        WEBSOCKETOBJECT.broadcastTXT(channelModeOutput1);
       // _output_summary += " [" + channelModeOutput1 + "]";
        clearADCScopeData1();
        return " ";
}     
        
  



void ADCInit(void) {
  byte internalError;
  byte ADCSetupByte = 210;
  byte ADCConfigByte = 97;
  Wire.beginTransmission(ADCAddress);
  Wire.write(ADCSetupByte);
  Wire.write(ADCConfigByte);
  internalError = Wire.endTransmission();
  if (internalError == 0) {
    Serial.println("i2c ADC Initialized");
  }
}
void setADCChannel(int CHANNEL) {
  byte internalError, ADCConfigByte;
  //Select correct channel
  switch (CHANNEL) {
    case 0:
      ADCConfigByte = 97;
      break;
    case 1:
      ADCConfigByte = 99;
      break;
    default:
      ADCConfigByte = 97;
      break;
  }
  //Send channel selection
  Wire.beginTransmission(ADCAddress);
  Wire.write(ADCConfigByte);
  internalError = Wire.endTransmission();
  if (internalError != 0) {
    Serial.println("Error setting ADC channel");
  }
}
int ADCRead(void) {
  //Read I2C ADC channel
  Wire.requestFrom(ADCAddress, 2);
  if (Wire.available() > 0) {
    byte ADCResultMSB = Wire.read();
    byte ADCResultLSB = Wire.read();
    uint16_t ADCResult = (((ADCResultMSB << 8) | ADCResultLSB) & 0x0FFF);
    return ADCResult;
  }
}

void fastADChandler(void){
  float scale0= 1024/3.3, scale1= 1024/3.3; 
  float scaleb=1.0; int   temp=0;
  long _lastsampletime = micros();
  String Mode1,Mode2;
  Mode1=getChanneMode1();
  Mode2=getChanneMode2();
  bool ADC1ON,ADC2ON,TRION1,TRION2;
  ADC1ON= (Mode1 == "INT ADC");
  ADC2ON= (Mode2 == "INT ADC");
  TRION1 = (Mode1 == "TRIANGLE");
  TRION2 = (Mode2 == "TRIANGLE");
   // checks for !Data_RTS before being called
   // Build up (FULL!) message for websock while RTS is false..
  while(NumberofSamplesRead <= MAX_Samples()) {  // max number of samples and as fast as possible? 
    while ( micros() <= getsampleuSTimer() +_lastsampletime ){ yield;  } //  pause to synch with sample rate
    _lastsampletime=micros();
    temp=0;buffer[0][NumberofSamplesRead]=0; buffer[1][NumberofSamplesRead]=0;buffer[2][NumberofSamplesRead]=0;
    TestTriangle1 = TestTriangle1+0.5;
    if ( TestTriangle1 >=330){TestTriangle1 =-330;}  // to fake +1-1 without changing scale facto
      
     if (ADC1ON) {buffer[0][NumberofSamplesRead]=analogRead(0);}
     if (ADC2ON) {
       if (ADC1ON) {buffer[1][NumberofSamplesRead]=buffer[0][NumberofSamplesRead];}
               else{buffer[1][NumberofSamplesRead]=analogRead(0);}
               }
     if (TRION1) {buffer[0][NumberofSamplesRead]=TestTriangle1;}
     if (TRION2) {buffer[1][NumberofSamplesRead]=TestTriangle1;}
        if (digitalRead(ScopeDigInput0) == 1) {
          temp = temp + 1;  // offset!
          }
        if (digitalRead(ScopeDigInput1) == 1) {
          temp = temp + 2;  // offset!
        } 
      buffer[2][NumberofSamplesRead]=temp;
      NumberofSamplesRead++;
      if (NumberofSamplesRead >= MAX_Samples()) {
        Set_Data_RTS(true); // stop doing it!
        // all read, so scale and send to the scopestring
          for (int sample=0;sample < NumberofSamplesRead;sample++){
               BuildScopeDataString(String((buffer[0][sample])/scale0, 3),String((buffer[1][sample])/scale1, 3),String((buffer[2][sample])/scaleb, 1 ));
            }
            
        Serial.print(" r");Serial.print(NumberofSamplesRead) ;  Serial.println(">");     
       
        }// limited to  number of samples to the screen width 
    } //while
      // NOW add datalog (first sample only) for fast samples?
     if ( getDataLog() ) {   // ?? Allow 
     sendTime = millis();
      Serial.println("");
      Serial.print("DL Time:");
      Serial.print(sendTime / 1000);
      Serial.print('.');
      Serial.print(sendTime % 1000); // debugging follows ...
      Serial.printf("CH1:  %.3f  CH2: %.3f  Dig %id  ",String((buffer[0][0])/scale0, 3),String((buffer[1][0])/scaleb, 3),String((buffer[2][0])/scaleb, 1 ));
      }
       //
  

  }//function
  




void ADCHandler(void) {  // NOW DUPLEX CH1/CH2 +digital third channel  ONLY reads BOTH channels and digitals and builds up the strings to send data in bulk
  float temp1,temp2;
  float _digital;
   if ( !Data_RTS() ) {  // Build up message for websock while RTS is false..
   
    temp1=ChannelRead1();temp2=ChannelRead2(); _digital= DigitalPortRead();
    
    BuildScopeDataString(String(temp1, 3),String(temp2, 3),String(_digital, 1 ));
   
    NumberofSamplesRead++;  
    if (every(NumberofSamplesRead,50) )  {Serial.print("-");} 
    //if (every(NumberofSamplesRead,50) )  {Serial.print(Data_RTS());Serial.println("-");}
    if (NumberofSamplesRead >= MAX_Samples()) {Set_Data_RTS(true);  Serial.println(">");       }// limit number of samples to the screen width 
    if (getsampleuSTimer() >= 5000) {Set_Data_RTS(true); 
       if (!getDataLog()){Serial.println(">"); }   // gives tidy <> in serial print (< is HasBeenSent )
    }
   } 
  if ((getDataLog()) && (getsampleuSTimer() >= 5000)) {   // ?? Allow faster update rate for slow samples per second
     sendTime = millis();
      Serial.println("");
      Serial.print("DL Time:");
      Serial.print(sendTime / 1000);
      Serial.print('.');
      Serial.print(sendTime % 1000); // debugging follows ...
      int _DIG = _digital; //integer it 
      Serial.printf("CH1:  %.3f  CH2: %.3f  Dig %id  ", temp1,temp2, _DIG );
   }
  
 }


float ChannelRead1(void) {
  float temp =0;
  String Mode;
  Mode=getChanneMode1();
  //Serial.println(Mode);
  if (Mode == "DIG") {
    temp=0;
    if (digitalRead(ScopeDigInput0) == 1) {
      temp = temp + 1;  // offset!
      }
    if (digitalRead(ScopeDigInput1) == 1) {
      temp = temp + 2;  // offset!
    } 
    CH1Scale = 1;
  }
  if (Mode == "INT ADC") {
   CH1Scale = 1024 / 3.3;  //3.3v ref, output in mv1024 not 2048
   temp = (float(analogRead(0)) / CH1Scale); // force float?
   }

  if (Mode == "SCALES") {
    CH1Scale = -100000 ;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ same AS 5 V! 
    temp = (readScales(0)/ CH1Scale);;
  }
  if (Mode == "SCALESB") {
    CH1Scale = -25000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
    temp = (readScales(1)/ CH1Scale);
  }
  if (Mode == "4V ADC") {
    CH1Scale = 2048 / 4;
    setADCChannel(0);
    temp = ((ADCRead() * 4096 / 64) / CH1Scale);
  }
  
  if (Mode == "TRIANGLE"){ 
    TestTriangle1 = TestTriangle1+0.01;
    if ( TestTriangle1 >=1){TestTriangle1 =-1;}
    temp = TestTriangle1 ;
    }
 //Serial.print("CH1 out:"); Serial.println(temp);  
  return temp;
}

float DigitalPortRead(void){
   float temp=0;
    if (digitalRead(ScopeDigInput0) == 1) {
      temp = temp + 1;  
      }
    if (digitalRead(ScopeDigInput1) == 1) {
      temp = temp + 2;  
    } 
    return temp;
}

float ChannelRead2(void) {
  float temp =0;
  String Mode;
  Mode = getChanneMode2();
  if (Mode == "DIG") {
    temp=0;
    if (digitalRead(ScopeDigInput0) == 1) {
      temp = temp + 1;  // offset!
      }
    if (digitalRead(ScopeDigInput1) == 1) {
      temp = temp + 2;  // offset!
    } 
    CH2Scale = 1;
  }
  if (Mode == "INT ADC") {
    CH2Scale = 1024 / 3.3;  //3.3v ref, output in V
    temp = (analogRead(0)) / CH2Scale;
  }

  if (Mode == "SCALES") {
    CH2Scale = -100000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V! 209 and 1000 2090 and 100 20900 and 10 here
    temp = (readScales(0)/ CH2Scale) ;
    }
  if (Mode == "SCALESB") {
    CH2Scale = -25000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
    temp = (readScales(1)/ CH2Scale);
  }
  if (Mode == "4V ADC") {
    CH2Scale = 2048 / 4;
    setADCChannel(0);
    temp = ((ADCRead() * 4096 / 64) / CH2Scale);
  }

if (Mode == "TRIANGLE") {
    TestTriangle = TestTriangle+0.01;
    if ( TestTriangle >= 1){TestTriangle = -1 ;}
    temp = TestTriangle ;
  }
  //Serial.print("CH2 out:"); Serial.println(temp);  
  return temp;
}
