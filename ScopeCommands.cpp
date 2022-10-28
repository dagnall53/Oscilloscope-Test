#include "ScopeCommands.h"
#include <HX711.h>

HX711 scale;


boolean ScalesPresent;
String channelModeOutput1;
String channelModeOutput2;
bool toggledChannelOffFlag1;
bool toggledChannelOffFlag2;
byte ADCAddress = 54;  //adc address on 12c
long offset = 64;
long CH1Scale = 1;  //DAG new variables for scaling everything
long CH2Scale = 1;
float CH1scaleset = -2.09;
float CH2scaleset = -2.09;
unsigned long sendTime = 0;
long TAREA = 0;
long TAREB = 0;
byte LastChanRead = 0;

int MAX_Samples = 400;   // for duplex testing

int NumberofSamplesRead = 0;
bool _getdata ;
bool _Request_SendWSest;

byte ScopeDigInput0, ScopeDigInput1;
int Screen_update_time;
int ScreenUpdate(int A) {
  Screen_update_time = A;
}
int Screen_U_time() {
  return Screen_update_time;
}

bool Request_Sample_Send (void){
  return _Request_SendWSest;
}
void Set_Request_Sample_Send (bool set) {
 _Request_SendWSest = set;
}


bool Data_RTS (void){
  return _getdata;
}
void Set_Data_RTS( bool set){
  _getdata = set;
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
  String _output_summary = "";

  if ((getChanneMode1() == "4V ADC") || (getChanneMode1() == "64V ADC") || (getChanneMode1() == "INT ADC") || (getChanneMode1() == "DIG") || (getChanneMode1() == "SCALES")) {
    toggledChannelOffFlag1 = false;
    
      if ((getDuplexMode()) ) { // !! Pling as we have managed to capture.. or another flag ? 
        channelModeOutput1 = "SCOPE ADC DUPLEX";
        channelModeOutput1 += String(getADCScopeData1());  // getADCScopeData1 has special DUPLEX mode that captures BOTH channels 
        NumberofSamplesRead =0 ; //  reset 
        } else { 
       // prevent Scope ADC channel 1 if not ready 
        channelModeOutput1 = "SCOPE ADC DATACHANNEL1";
        channelModeOutput1 += String(getADCScopeData1());
      }
    
   

    WEBSOCKETOBJECT.broadcastTXT(channelModeOutput1);
    _output_summary += " [" + channelModeOutput1 + "]";
  }
  if (getChanneMode1() == "UART") {
    toggledChannelOffFlag1 = false;
    channelModeOutput1 = "SCOPE UART DATACHANNEL1";
    channelModeOutput1 += getUartScopeData();
    WEBSOCKETOBJECT.broadcastTXT(channelModeOutput1);
    _output_summary += " [" + channelModeOutput1 + "]";
    clearUartScopeData();
  }
  if (getChanneMode1() == "OFF") {
    if (!toggledChannelOffFlag1) {
      toggledChannelOffFlag1 = true;
      channelModeOutput1 = "SCOPE OFF DATACHANNEL1";
      channelModeOutput1 += " 0";
      WEBSOCKETOBJECT.broadcastTXT(channelModeOutput1);
      _output_summary += " [" + channelModeOutput1 + "]";
    }
  }

  // for test only Serial.println(channelModeOutput1);
  clearADCScopeData1();
  //Channel 2

  if ((getChanneMode2() == "4V ADC") || (getChanneMode2() == "64V ADC") || (getChanneMode2() == "INT ADC") || (getChanneMode2() == "DIG") || (getChanneMode2() == "SCALESB") || (getChanneMode2() == "SCALES")) {
    toggledChannelOffFlag2 = false;
    if (!getDuplexMode()) {
    channelModeOutput2 = "SCOPE ADC DATACHANNEL2";
    channelModeOutput2 += String(getADCScopeData2());
    WEBSOCKETOBJECT.broadcastTXT(channelModeOutput2);
    _output_summary += " [" + channelModeOutput2 + "]";
    }
  }


  if (getChanneMode2() == "UART") {
    toggledChannelOffFlag2 = false;
    channelModeOutput2 = "SCOPE UART DATACHANNEL2";
    channelModeOutput2 += getUartScopeData();
    WEBSOCKETOBJECT.broadcastTXT(channelModeOutput2);
    _output_summary += " [" + channelModeOutput2 + "]";
    clearUartScopeData();
  }
  if (getChanneMode2() == "OFF") {
    if (!toggledChannelOffFlag2) {
      toggledChannelOffFlag2 = true;
      channelModeOutput2 = "SCOPE OFF DATACHANNEL2";
      channelModeOutput2 += " 0";
      WEBSOCKETOBJECT.broadcastTXT(channelModeOutput2);
      _output_summary += " [" + channelModeOutput2 + "]";
    }
  }
  // for test only Serial.println(channelModeOutput2);
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
  //Read channel
  Wire.requestFrom(ADCAddress, 2);
  if (Wire.available() > 0) {
    byte ADCResultMSB = Wire.read();
    byte ADCResultLSB = Wire.read();
    uint16_t ADCResult = (((ADCResultMSB << 8) | ADCResultLSB) & 0x0FFF);
    return ADCResult;
  }
}
void ADCHandler(void) {  // reads BOTH channels and sets the strings
  
 
  if (getDuplexMode() && !Data_RTS() ) {  
    addADCScopeData1(String(ChannelRead1(), DEC)); addADCScopeData1(String(ChannelRead2(), DEC));
    NumberofSamplesRead++;  
    if (NumberofSamplesRead >= MAX_Samples) {Set_Data_RTS(true);}// limit number of samples that can be sent in a duplex websock message
  } else 
  { 
    addADCScopeData1(String(ChannelRead1(), DEC));
    addADCScopeData2(String(ChannelRead2(), DEC)); 
  }
  
}


long ChannelRead1(void) {
  long temp;
  if (getChanneMode1() == "DIG") {
    if (digitalRead(ScopeDigInput0) == 1) {
      temp = 128;  // offset!
    } else {
      temp = 96;
    }
    CH1Scale = 1;
  }
  if (getChanneMode1() == "INT ADC") {
    CH1Scale = 1024 / 3.3;  //3.3v ref, output in mv1024 not 2048
    temp = (analogRead(0) * 4096 / 64) / CH1Scale;
  }

  if (getChanneMode1() == "SCALES") {
    CH1Scale = -100000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V! 209 and 1000 2090 and 100 20900 and 10 here
    temp = readScales(0);
    temp = ((temp * 4096 / 64) / CH1Scale) + offset;
  }
  if (getChanneMode1() == "SCALESB") {
    CH1Scale = -25000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
    temp = readScales(1);
    temp = ((temp * 4096 / 64) / CH1Scale) + offset;
  }
  if (getChanneMode1() == "4V ADC") {
    CH1Scale = 2048 / 4;
    setADCChannel(0);
    temp = ((ADCRead() * 4096 / 64) / CH1Scale);
  }
  return temp;
}

long ChannelRead2(void) {
  long temp;
  if (getChanneMode2() == "DIG") {
    if (digitalRead(ScopeDigInput0) == 1) {
      temp = 128;  // offset!
    } else {
      temp = 96;
    }
    CH2Scale = 1;
  }
  if (getChanneMode2() == "INT ADC") {
    CH2Scale = 1024 / 3.3;  //3.3v ref, output in mv1024 not 2048
    temp = (analogRead(0) * 4096 / 64) / CH1Scale;
  }

  if (getChanneMode2() == "SCALES") {
    CH1Scale = -100000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V! 209 and 1000 2090 and 100 20900 and 10 here
    temp = readScales(0);
    temp = ((temp * 4096 / 64) / CH1Scale) + offset;
  }
  if (getChanneMode2() == "SCALESB") {
    CH1Scale = -25000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
    temp = readScales(1);
    temp = ((temp * 4096 / 64) / CH1Scale) + offset;
  }
  if (getChanneMode2() == "4V ADC") {
    CH1Scale = 2048 / 4;
    setADCChannel(0);
    temp = ((ADCRead() * 4096 / 64) / CH1Scale);
  }


  return temp;
}




void ADCHandler(byte chan) {
  long temp;
  bool D0, D1;
  if ((chan == 1) || (chan == 0)) {
    // clearADCScopeData1();
    if (getChanneMode1() == "DIG") {
      if (digitalRead(ScopeDigInput0) == 1) {
        temp = 128;  // offset!
      } else {
        temp = 96;
      }
      addADCScopeData1(String(temp, DEC));
      CH1Scale = 1;
    }
    if (getChanneMode1() == "INT ADC") {
      CH1Scale = 1024 / 3.3;  //3.3v ref, output in mv1024 not 2048
      temp = (analogRead(0) * 4096 / 64) / CH1Scale;
      addADCScopeData1(String(temp, DEC));
    }

    if (getChanneMode1() == "SCALES") {
      CH1Scale = -100000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V! 209 and 1000 2090 and 100 20900 and 10 here
      temp = readScales(0);
      temp = ((temp * 4096 / 64) / CH1Scale);
      addADCScopeData1(String(temp + offset, DEC));  // add offset here, keeps temp as the actual reading
    }
    if (getChanneMode1() == "SCALESB") {
      CH1Scale = -25000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
      temp = readScales(1);
      temp = ((temp * 4096 / 64) / CH1Scale);
      addADCScopeData1(String(temp + offset, DEC));  // add offset here, keeps temp as the actual reading
    }
    if (getChanneMode1() == "4V ADC") {
      CH1Scale = 2048 / 4;
      setADCChannel(0);
      temp = ((ADCRead() * 4096 / 64) / CH1Scale);
      addADCScopeData1(String(temp, DEC));
    }
    if (getChanneMode1() == "64V ADC") {
      CH1Scale = 2048 / 64;
      setADCChannel(1);
      temp = ((ADCRead() * 4096 / 64) / CH1Scale);
      addADCScopeData1(String(temp, DEC));
    }
    if (getDataLog()) {
      sendTime = millis();
      Serial.println("");
      Serial.print("DL Time:");
      Serial.print(sendTime / 1000);
      Serial.print('.');
      Serial.print(sendTime % 1000);
      if (getChanneMode1() != "OFF") {
        if (getChanneMode1() != "SCALES") {
          Serial.print(" CHANNEL1 mV : ");
        } else {
          Serial.print(" CHANNEL1 grams: ");
        }
        Serial.print((temp * 1000 / 64));
      }
    }
  }  //end of if ch 1

  if ((chan == 2) || (chan == 0)) {
    // clearADCScopeData2();
    if (getChanneMode2() == "DIG") {
      // Serial.println(digitalRead(13));  //test
      if (digitalRead(ScopeDigInput1) == 1) {
        temp = 32;
      } else {
        temp = 0;
      }
      addADCScopeData2(String(temp, DEC));
      CH2Scale = 1;
    }
    if (getChanneMode2() == "INT ADC") {
      CH2Scale = 1024 / 3.3;  //3.3v ref, output in mv1024 not 2048
      temp = (analogRead(0) * 4096 / 64) / CH2Scale;
      addADCScopeData2(String(temp, DEC));
    }
    if (getChanneMode2() == "SCALES") {
      CH2Scale = -100000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
      temp = readScales(0);
      temp = ((temp * 4096 / 64) / CH2Scale);
      addADCScopeData2(String(temp + offset, DEC));
    }
    if (getChanneMode2() == "SCALESB") {
      CH2Scale = -25000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
      temp = readScales(1);
      temp = ((temp * 4096 / 64) / CH2Scale);
      addADCScopeData2(String(temp + offset, DEC));
    }
    if (getChanneMode2() == "4V ADC") {
      setADCChannel(0);
      CH2Scale = 2048 / 4;
      temp = ((ADCRead() * 4096 / 64) / CH2Scale);
      addADCScopeData2(String(temp, DEC));
    }
    if (getChanneMode2() == "64V ADC") {
      setADCChannel(1);
      CH2Scale = 2048 / 64;
      temp = ((ADCRead() * 4096 / 64) / CH2Scale);
      addADCScopeData2(String(temp, DEC));
    }
    if (getDataLog()) {
      if (getChanneMode2() != "OFF") {
        if ((getChanneMode2() == "SCALESB") || (getChanneMode2() == "SCALES")) {
          Serial.print("   CHANNEL2 grams, ");
        } else {
          Serial.print("   CHANNEL2 mV, ");
        }
        Serial.print(temp * 1000 / 64);
      }
    }
  }                   //end of if channelmode = 2
  if ((chan == 3)) {  // simultaneous read digital test first
    D0 = digitalRead(ScopeDigInput0);
    D1 = digitalRead(ScopeDigInput1);
    CH1Scale = 1024 / 3.3;  //3.3v ref, output in mv1024 not 2048
    temp = (analogRead(0) * 4096 / 64) / CH1Scale;
    if (getChanneMode1() == "SCALES") {
      CH1Scale = -100000;  //DAG NB set in initscales to grammes, 5KG EXPECTED TO READ AS 5 V!
      temp = readScales(0);
      temp = ((temp * 4096 / 64) / CH1Scale);
      addADCScopeData1(String(temp + offset, DEC));  // add offset here, keeps temp as the actual reading
    }
    if (getChanneMode1() == "INT ADC") {
      addADCScopeData1(String(temp, DEC));
    }
    if (getChanneMode2() == "INT ADC") {
      addADCScopeData2(String(temp, DEC));
    }
    if (getChanneMode1() == "DIG") {
      if (D0) {
        temp = 128;  // offset!
      } else {
        temp = 96;
      }
      addADCScopeData1(String(temp, DEC));
      CH1Scale = 1;
    }
    if (getChanneMode2() == "DIG") {
      if (D1) {
        temp = 32;  // offset!
      } else {
        temp = 0;
      }
      addADCScopeData2(String(temp, DEC));
      CH1Scale = 1;
    }
  }
}
