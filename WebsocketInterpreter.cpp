#include "WebsocketInterpreter.h"



extern void handleTest();
void webSocketDataInterpreter(WebSocketsServer &WEBSOCKETOBJECT, String WEBSOCKETDATA)
{
  String topLevelToken = "";
  String subLevelToken = "";
  String serialClear = "SERIAL UART CLEAR";
//  Serial.println("New data received: " + WEBSOCKETDATA); //  turn on for debugging!


  
 if(WEBSOCKETDATA.startsWith("Data_accepted"))
  { 
  //  Serial.println(" Data Accepted reset RTS so we can read more.. ");
    Set_Data_RTS (false); 
    //Set_CTS(false);
  }

 if (WEBSOCKETDATA.startsWith("Clear_to_send")){
  // Serial.println(" _CTS ");
    Set_CTS(true);

 }
if(WEBSOCKETDATA.startsWith("SERIAL_BAUD"))
  { 
   topLevelToken = "SERIAL_BAUD";
   SetBaud(WEBSOCKETDATA.substring(topLevelToken.length()+1).toInt());
  }



if(WEBSOCKETDATA.startsWith("SERIAL_OUT"))
  { 
   topLevelToken = "SERIAL_OUT";
   Serial.println ( WEBSOCKETDATA.substring(topLevelToken.length()+1));
  }

  //I2C related tasks
  if(WEBSOCKETDATA.startsWith("I2C"))
  {
    //Look at start of line for tokens, add +1 to length to account for space
    topLevelToken = "I2C";
    String i2cCommand = WEBSOCKETDATA.substring(topLevelToken.length()+1);
    if(i2cCommand.startsWith("SCAN"))
    {
      WEBSOCKETOBJECT.broadcastTXT(serialClear);
      scanI2CAddressAndRegisters(WEBSOCKETOBJECT);
    }
    if(i2cCommand.startsWith("FIND DEVICES"))
    {
      WEBSOCKETOBJECT.broadcastTXT(serialClear);
      scanI2CAddress(WEBSOCKETOBJECT);
    }    
    if(i2cCommand.startsWith("READ DEVICE"))
    {
      WEBSOCKETOBJECT.broadcastTXT(serialClear);
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "READ DEVICE";
      byte deviceToRead = i2cCommand.substring(subLevelToken.length()+1).toInt();
      scanI2CRegisters(WEBSOCKETOBJECT, deviceToRead);
    }
  }
  //Oscilloscope related tasks
  if(WEBSOCKETDATA.startsWith("SCOPE"))
  {
    //Look at start of line for tokens, add +1 to length to account for space
    topLevelToken = "SCOPE";
    String scopeCommand = WEBSOCKETDATA.substring(topLevelToken.length()+1);
   
   if(scopeCommand.startsWith("DUPLEX 1"))
    {
      //Look at start of line for subtokens, add +1 to length to account for space
      //subLevelToken = "DUPLEX 1";
      setDuplexMode(true);
      subLevelToken = "CHANNEL 1";
      setChannelMode1(scopeCommand.substring(subLevelToken.length()+1));
      } 
   if(scopeCommand.startsWith("DUPLEX 2")) {
      setDuplexMode(true);
      subLevelToken = "CHANNEL 2";
      setChannelMode2(scopeCommand.substring(subLevelToken.length()+1));
   }

    
    if(scopeCommand.startsWith("CHANNEL 1"))
    {
      setDuplexMode(false);
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "CHANNEL 1";
      setChannelMode1(scopeCommand.substring(subLevelToken.length()+1));
     }

    if(scopeCommand.startsWith("CHANNEL 2"))
    {
      setDuplexMode(false);
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "CHANNEL 2";
      setChannelMode2(scopeCommand.substring(subLevelToken.length()+1));
    }
    
    
    if(scopeCommand.startsWith("WS_Timer"))
    {
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "WS_Timer";  //DAG the scope "timescale"
      setWS_Timer(scopeCommand.substring(subLevelToken.length()+1).toInt());
    }
    if(scopeCommand.startsWith("Sample_uS"))
    {
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "Sample_uS";  //DAG added to directly set Sample_uS 
      setsampleuSTimer(scopeCommand.substring(subLevelToken.length()+1).toInt());
    }

    if(scopeCommand.startsWith("PAUSE"))
    {
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "PAUSE";
      if(scopeCommand.substring(subLevelToken.length()+1) == "ON")
      {
        setPAUSE(true);
      }
      else
      {
        setPAUSE(false);
      }
    }



    if(scopeCommand.startsWith("DATALOG"))
    {
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "DATALOG";
      if(scopeCommand.substring(subLevelToken.length()+1) == "ON")
      {
        setDataLog(true);
      }
      else
      {
        setDataLog(false);
      }
    }
    
    if(scopeCommand.startsWith("TIMESCALE"))
    {
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "DATALOG";
      Serial.print("set timescale");
      Serial.println(scopeCommand.substring(subLevelToken.length()+1).toInt());
    }
     if(scopeCommand.startsWith("TARE"))
    {
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "TARE"; //dag NOT IMPLEMENTED in webpage, as HX711 does a TARE on startup.
      if(scopeCommand.substring(subLevelToken.length()+1) == "ON")
      {
        setTARE(true);   // Although not doing anything else yet, the button does change....
      }
      else
      {
        setTARE(false);
      }
    }
  }
  //Stripchart related tasks
  if(WEBSOCKETDATA.startsWith("Strip"))   // comes from 
  {
    //Look at start of line for tokens, add +1 to length to account for space
    topLevelToken = "STRIP";
    String terminalCommand = WEBSOCKETDATA.substring(topLevelToken.length()+1);
    if(terminalCommand.startsWith("Chart"))
    {
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "CHART";
      if(terminalCommand.substring(subLevelToken.length()+1) == "Selected")
      { 
       // handleTest();   // start alternate web display? not working 
      }
      else
      {
        //DISCONNECT FROM SERIAL
      }
    }
  }

  //Terminal related tasks
  if(WEBSOCKETDATA.startsWith("TERMINAL"))
  {
    //Look at start of line for tokens, add +1 to length to account for space
    topLevelToken = "TERMINAL";
    String terminalCommand = WEBSOCKETDATA.substring(topLevelToken.length()+1);
    if(terminalCommand.startsWith("CONNECT"))
    {
      //Look at start of line for subtokens, add +1 to length to account for space
      subLevelToken = "CONNECT";
      if(terminalCommand.substring(subLevelToken.length()+1) == "ON")
      {
        //CONNECT TO SERIAL
      }
      else
      {
        //DISCONNECT FROM SERIAL
      }
    }
  }
}
