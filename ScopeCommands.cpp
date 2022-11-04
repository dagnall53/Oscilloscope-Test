#include "ScopeCommands.h"
#include <HX711.h>

HX711 scale;


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

int MAX_Samples = 320;   // for duplex testing testd to 1000 not found limit. should be set to number across the screen! . 

int NumberofSamplesRead = 0;
float TestTriangle = 0;
float TestTriangle1 =0;
bool _RTS ;
bool _CTS;

byte ScopeDigInput0, ScopeDigInput1;
int Screen_update_time;
int ScreenUpdate(int A) {
  Screen_update_time = A;
}
int Screen_U_time() {
  return Screen_update_time;
}

bool Read_CTS (void){
  return _CTS;
}
void Set_CTS (bool set) {
 _CTS = set;
}


bool Data_RTS (void){
  return _RTS;
}
void Set_Data_RTS( bool set){
  // if false also reset Numberof samples read counter ?
  _RTS = set;
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

      if (Chan != LastChanRead) {
        scale.set_gain(128);
        Reading = scale.read();
      }  //dummy to get ready because this needs setting up
      //CH1scaleset);  this value is obtained by calibrating the scale with known weights; see the README for details
      LastChanRead = Chan;
      Reading = (((scale.read_average(average) - TAREA) / CH1scaleset));
    }
    if (Chan == 1) {  // CH B
      if (Chan != LastChanRead) {
        scale.set_gain(32);
        Reading = scale.read();
      }  //dummy to get ready
         //CH2scaleset);  This value is obtained by calibrating the scale with known weights; see the README for details
      LastChanRead = Chan;
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
  //   Serial.print("ADCdata1");
  // Serial.println(getADCScopeData1());
  // Serial.print("    ADCdata2");
  //Serial.println(getADCScopeData2());
  //Serial.print("scopehandler getchannelmode1[");Serial.print(getChanneMode1());Serial.println("]");
  String _output_summary = "";
// only doing "duplex mode" now.. 
        //Serial.print("duplex send");
        channelModeOutput1 = "SCOPE ADC DUPLEX";
        channelModeOutput1 += String(getADCScopeData1());  // getADCScopeData1 has special DUPLEX mode that captures BOTH channels 
        WEBSOCKETOBJECT.broadcastTXT(channelModeOutput1);
        _output_summary += " [" + channelModeOutput1 + "]";
        clearADCScopeData1();
        clearADCScopeData2();        
        return _output_summary;
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
void ADCHandler(void) {  // DUPLEX MODE ONLY reads BOTH channels and builds up the strings to send data in bulk
  float temp1,temp2;
  //if (getDuplexMode() && !Data_RTS() ) {  // Build up while RTS is false..
   if ( !Data_RTS() ) {  // Build up message for websock while RTS is false..
    temp1=ChannelRead1();temp2=ChannelRead2();
    addADCScopeData1(String(temp1, DEC));  addADCScopeData1(String(temp2, DEC));
    NumberofSamplesRead++;  
  //  if (every(NumberofSamplesRead,50) )  {Serial.print("-");} 
    if (every(NumberofSamplesRead,50) )  {Serial.print(Data_RTS());Serial.println("-");}
    if (NumberofSamplesRead >= MAX_Samples) {Set_Data_RTS(true);}// limit number of samples that can be sent in a duplex websock message
    if (getsampleuSTimer() >= 5000) {Set_Data_RTS(true);} // ?? Allow faster update rate for slow samples per second
  } 
  if ((getDataLog()) && (getsampleuSTimer() >= 5000)) {   
     sendTime = millis();
      Serial.println("");
      Serial.print("DL Time:");
      Serial.print(sendTime / 1000);
      Serial.print('.');
      Serial.print(sendTime % 1000); // debugging follows ...
      Serial.print("  #");Serial.print(NumberofSamplesRead);Serial.print(" RTS(");Serial.print(Data_RTS());Serial.print(") ");
      Serial.print(" Duplex(");Serial.print(getDuplexMode());Serial.print(") ");
      if (getChanneMode1() != "OFF") {
        if (getChanneMode1() != "SCALES") {
          Serial.print(" CHANNEL1 V : ");
        } else {
          Serial.print(" CHANNEL1 Kg: ");
        }
        Serial.print(temp1);      // SENDING "Truth in floating point"
      }
      if (getChanneMode2() != "OFF") {
        if ((getChanneMode2() == "SCALESB") || (getChanneMode2() == "SCALES")) {
          Serial.print("   CHANNEL2 Kg: ");
        } else {
          Serial.print("   CHANNEL2 V: ");
        }
        Serial.print(temp2);      // SENDING "Truth in floating point"
      }
     }
  
 }


float ChannelRead1(void) {
  float temp;
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
    temp = readScales(0);
    temp = (temp  / CH1Scale);
  }
  if (Mode == "SCALESB") {
    CH1Scale = -25000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
    temp = readScales(1);
    temp = ((temp ) / CH1Scale);
  }
  if (Mode == "4V ADC") {
    CH1Scale = 2048 / 4;
    setADCChannel(0);
    temp = ((ADCRead() * 4096 / 64) / CH1Scale);
  }
  
  if (Mode == "TRIANGLE"){ 
    //Serial.println("mode is triangle");
    
    TestTriangle1 = TestTriangle1+0.1;
    if ( TestTriangle1 >=5){TestTriangle1 =-5;}
    temp = TestTriangle1 ;
    }
 //Serial.print("CH1 out:"); Serial.println(temp);  
  return temp;
}

float ChannelRead2(void) {
  float temp;
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
    CH1Scale = 1;
  }
  if (Mode == "INT ADC") {
    CH2Scale = 1024 / 3.3;  //3.3v ref, output in V
    temp = (analogRead(0)) / CH1Scale;
  }

  if (Mode == "SCALES") {
    CH1Scale = -100000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V! 209 and 1000 2090 and 100 20900 and 10 here
    temp = readScales(0);
    temp = ((temp ) / CH1Scale) ;
  }
  if (Mode == "SCALESB") {
    CH1Scale = -25000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
    temp = readScales(1);
    temp = ((temp ) / CH1Scale);
  }
  if (Mode == "4V ADC") {
    CH1Scale = 2048 / 4;
    setADCChannel(0);
    temp = ((ADCRead() * 4096 / 64) / CH1Scale);
  }



if (Mode == "TRIANGLE") {
    
    
    TestTriangle = TestTriangle+0.01;
    if ( TestTriangle >= 1){TestTriangle = -1 ;}
    temp = TestTriangle ;
  }

  return temp;
}


