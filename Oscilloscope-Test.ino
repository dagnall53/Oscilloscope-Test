#include <Arduino.h>
/// NodeMCU numbering //this update: 08 July 2017
/*  / these are defined in arduino
  static const uint8_t D0   = 16;  and Red Led on NodeMcu V2 (not present on NodeMCU v3)
  static const uint8_t D1   = 5;
  static const uint8_t D2   = 4;
  static const uint8_t D3   = 0;
  static const uint8_t D4   = 2;  and Blue Led on SP8266
  static const uint8_t D5   = 14;
  static const uint8_t D6   = 12;
  static const uint8_t D7   = 13;
  static const uint8_t D8   = 15;
  static const uint8_t D9   = 3;
  static const uint8_t D10  = 1;
  #define BlueLed 2 // NB  same as PIN D4!
 
 // This update 13:11 17 August 2017

*/
// tested WEMOS / LOLIN D32

static const uint8_t LED = 16;
#ifdef ESP32  
  #include <ESPmDNS.h>
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
 #else
  #include <ESP8266WiFi.h>
  #include <ESP8266WiFiMulti.h>

  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
#endif


#include <WebSocketsServer.h>


#include <WiFiClient.h>
#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <WiFiManager.h>  // WIFI Manager by tzapu (tested 2.0.14)
#include <Wire.h>
#include "ScopeCommands.h"
//#include "miniDB.h" // called from scope commands and websocket interprete
#include "webStripChart.h"
#include "WebsocketInterpreter.h"
#include "OTA_Web.h"

#ifdef ESP32  
// ESP32 ADC1 = 32-35  AVOID  36 and 39 as these are related to Hall
// ESPadc2 ADC2= 25,26,14,12,13,4,2,15,0  (Do not work ?? needs work to explore ) 
byte CH1A= 35;               // see line 171 (~)
byte CH2A= 32;
byte APMODE_BOOT_PIN = 13;  //DAG  press this pin to ground to start in AP mode..
byte D_in1 = 15;            //DAG
byte D_in2 = 2;             //DAG
String string_in1 = "(15)";
String string_in2 = "(2)";
byte _SDA = 12;   
byte _SCL = 14;   
byte _Data = 22;  // for hx 711
byte _Clock = 19;  // hx711
#else
// ESP8266 pins.. (see Scopecommands.h)

byte CH1A= 0;
byte CH2A= 0;
byte APMODE_BOOT_PIN = D7;  //DAG  press this pin to ground to start in AP mode..
byte D_in1 = D7;            //DAG
byte D_in2 = D6;            //DAG
String string_in1 = "(D7)";
String string_in2 = "(D6)";
byte _SDA = D4;   //05
byte _SCL = D5;   //04
byte _Data = D2;  // for hx 711
byte _Clock = D1;  // hx711
#endif

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void handleRoot();
void handleNotFound();
void serialEvent();

boolean bootAPFlag = true;

String inputString = "";
int uartIntData = -1;
boolean stringComplete = false;
String webSocketData = "";

unsigned long oldTime = 0;
unsigned long oldTimeADC = 0;
unsigned long currentTime = 0;
unsigned long LastSampleTime = 0;

//DAG added
String SettingsData;
boolean PHASE;
boolean ADC1READ;
String localIPaddr = "";
// DAG end

const char *ssid = "scope";
const char *password = "12345678";

MDNSResponder mdns;

//ESP8266WiFiMulti WiFiMulti;

#ifdef ESP32  
WebServer server(80);
#else
ESP8266WebServer server(80);
#endif


WebSocketsServer webSocket = WebSocketsServer(81);

void BROADCAST(String MSG);

void setup() {
  
  SetDigInputs(D_in1, D_in2);
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);  //DAG Turn on the blue LED
  delay(1000);
  digitalWrite(LED, 1);
  pinMode(APMODE_BOOT_PIN, INPUT_PULLUP);
  //ScreenUpdate(5000) ;
  //Serial.print(" Set up screen refresh at "); Serial.println (Screen_U_time());
  Serial.println("********************");
  Serial.printf("  Boot select pin(%i)  is <%i> \r\n",APMODE_BOOT_PIN,digitalRead(APMODE_BOOT_PIN) );
  if (!digitalRead(APMODE_BOOT_PIN)) {
    WiFi.disconnect();
    WiFi.softAP(ssid, password);
    Serial.println();
    Serial.println("Booting in AP mode");
    Serial.println("Go to 192.168.4.1 to access the oscilloscope");

  } else {
    Serial.println();
    Serial.println("Booting in client mode");
    WiFiManager wifiManager;
    wifiManager.autoConnect(ssid, password);
    LEDFLASH();
    delay(500);
    Serial.println("*****************");
    Serial.print("IP address: ");
    LEDFLASH();
    WiFi.softAP(ssid, password);  // and keep standard ap on 192.168.4.1
    delay(500);
    Serial.println("*****************");
    if (!MDNS.begin("scope")) {
      Serial.println("Error setting up MDNS responder!");
      while (1) {
        delay(1000);
      }
    } else { Serial.println("mDNS responder started");}
    LEDFLASH();
    Serial.print("Connect to http://scope.local or http://");
    Serial.println(WiFi.localIP());

    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
  }
  digitalWrite(LED, 1);  //DAG led OFF?
  SETESP32ANALOG(CH1A,CH2A);
  
  WebserverSetup();

  Wire.begin(_SDA, _SCL);
  scopeInit();
  setsampleuSTimer(5000);  //us = 5ms
  SetScalesConnected(0);
  //Serial.printf("Hall zero is %f ", SetHallZero());
  currentTime = millis();
  LastSampleTime = currentTime;
  SettingsData = "";
  Serial.println("testing for Connected I2C devices");
  scanI2CAddress(webSocket);
  Serial.println(SinglescanI2CAddress(webSocket, 60));
  Serial.println(SinglescanI2CAddress(webSocket, 50));
  Serial.println("testing for HX 711   ");
  ScalesInit(_Data, _Clock);  // needs to still work if no hx711!
  if (ScalesConnected() ) {Serial.println(" HX711 connected ");}else{Serial.println("NO SCALES Fitted");}
  ADC1READ = 0;
  Serial.println("Waiting for browser to connect");
  Serial.print("Connect to http://Oscilloscope.local or http://");
    Serial.println(WiFi.localIP());
  clearADCScopeData1();
  clearADCScopeData2();
  setPAUSE(true);  // do not start ADC immediately!
  DoingOTA(false);
  Set_Data_RTS (false);
 
}

void BROADCAST(String MSG) {
  webSocket.broadcastTXT(MSG);
}

void LEDFLASH(void) {
  digitalWrite(LED, PHASE);  //DAG LED flashing
  PHASE = !PHASE;
}

void loop() {
  currentTime = millis();
  serialEvent();
  #ifdef ESP32 
  #else
  MDNS.update(); //was originally called (HIDDEN!) by ArduinoOTA!!
  #endif
  webSocket.loop();
  server.handleClient();
  //Original
  if (  (currentTime - oldTimeADC) >= (getsampleuSTimer()/1000) )  {   //  sample rate is sent in us  BUT ONLY sample if Browser is OK to recieve!
    oldTimeADC = currentTime;_StartTestTimers();// _printStatus("\r\n Inside ADC call");
    if (!PAUSE() && !Data_RTS() && ACK() ){                // ACK is FROM the BROWSER - so do not send until it has been recieved
      _SetACK(false);
//Serial.print("Start ADC read");_Mark_Time(0,"StartADC");
      if (getsampleuSTimer() <=2100){
        fastADChandler();}  // Run faster, sends multiple samples as a websock group but  with just one adc and digital 
        else{ADCHandler();} //  duplex +digital, multi sources 
        SetHBS(false);  _SetACK(false);
        //_Mark_Time(1,"ADC Done");
        //_printStatus("--Just ended ADC call");
        // _printalltimes();  // use this timer to see how long it actually takes to get samples.. 
       //Serial.printf(" ADC handler %i   %i    %i us",_PrintTime(0),_PrintTime(1),_PrintTime(2));
     }
  }

  if (webSocketData != "") {
    webSocketDataInterpreter(webSocket, webSocketData);
   // Serial.println("Websocket data Handle");
    webSocketData = "";
  }
   //_printStatus("218");
  if ( !PAUSE()  && Data_RTS() && !HBS() ){ // !HBS() or !ACK?//  DATA RTS() is from the scope adc readings so send the reading but only once!
     //Serial.print("/");Serial.print(readNumberofsamplesRead());
    //Serial.println(scopeHandler(webSocket));  // scopehandler returns the sent data 
   
    //_printStatus("Starting Scopehandler ");
    LEDFLASH(); 
    scopeHandler(webSocket);
    webSocketData = "";
    oldTime = millis(); 
    delay(1); 
    SetHBS(true);  //dont send twice ?
    //_Mark_Time(2,"Scopehandler done");
    //_printStatus(" FINISHED Scopehandler ");// resets for new samples
     // NOW... wait for webbrowser to acknowledge?
   } 


}


#ifdef ESP32
void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
 const uint8_t* src = (const uint8_t*) mem;
 Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
 for(uint32_t i = 0; i < len; i++) {
   if(i % cols == 0) {
  Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
  }
  Serial.printf("%02X ", *src);
  src++;
  }
  Serial.printf("\n");

}
#endif


void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      if (!OTA_ON()){
        webSocket.close();
        WebserverSetup();
        Serial.printf("[%u] ATTEMPTING RESTART OF WEBSERVER!\r\n", num);  // not compatible with  OTA?
      }
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      webSocketData = String((const char *)payload);
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      
      hexdump(payload, length);   /// from debug.h in esp8266
     
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    case WStype_PONG:
      Serial.printf( "WStype [%d] is PONG\r\n",type );
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);  // see http://www.martyncurrey.com/esp8266-and-the-arduino-ide-part-9-websockets/
      break;
  }
}

void WebserverSetup() {
  server.on("/", handleStrip);
  //server.on("/HOME", handleRoot);
  server.on("/DATA", SendDATA);
  server.on("/data", SendDATA);
  // server.on("/TEST", handleTest);
  server.on("/STRIP", handleStrip);
  server.onNotFound(handleNotFound);
  // not yet.. server.on("/ota", handleOTA);  / will change to this format later ? 
  OTASettings();   
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void handleRoot() {
 // server.send_P(200, "text/html", INDEX_HTML);
}

// void handleTest() {
//   server.send_P(200, "text/html", Chart2);
// }
void handleStrip() {
  server.send_P(200, "text/html", STRIP_HTML);
}
void handleOTA() {
 //not ready yet server.send_P(200, "text/html", OTA_HTML);
}

void SendDATA() {
  server.sendContent(HTML_DATATEST());
  server.sendContent("");
  server.client().stop();
}

String HTML_DATATEST() {
  String st = "<div class=\"div2\">";
  st += "<p style=\"text-align:center;color:red\">*****TEST PAGE*****</p>";
  st += "<br>\r\n";

  st += "<br><b>WiFi DATA: </b>";
  st += "<br><b>SSID: </b>" + String(ssid);
  st += "<br><b>PASSWORD: </b>" + String(password);
  st += "<br><b>Network Accessible IP ADDRESS: </b>" + String(WiFi.localIP().toString());
  st += "<br><br>";
  st += "<input style=\"cursor:pointer; font-size:80%;\" type=\"button\" onclick=\"window.location.href ='/HOME'\" value=  HOME ></div>";
  st += "<br><input style=\"cursor:pointer; font-size:80%;\" type=\"button\" onclick=\"window.location.href ='/.'\" value=  Return ></div>";
  // st += "<br><input style=\"cursor:pointer; font-size:80%;\" type=\"button\" onclick=\"window.location.href ='/TEST'\" value= ' Goto TEST HTML - STRIP Chart '></div>";
  st += "<br><input style=\"cursor:pointer; font-size:80%;\" type=\"button\" onclick=\"window.location.href ='/STRIP'\" value= ' Goto PicoGraph demo Chart '></div>";
  // st += "<br><input style=\"cursor:pointer; font-size:80%;\" type=\"button\" onclick=\"window.location.href ='/SeaTalk1'\" value=  ADDITIONAL_SEATALK1_SETTINGS + ></div>";


  st += "<br><br>\r\n";
  st += "ADD NOTES HERE <br><br>\r\n";
  st += "<form>\r\n";
  st += "<center><input class=\"but\" style=\"width:20%;\" type=\"button\" onclick=\"window.location.href ='/.'\" value=\"STOP\"></center>\r\n";
  st += "</form><br><br>\r\n";
  st += "</div></body></html>\r\n";
  return st;
}







void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  // message += "<input style=\"cursor:pointer; font-size:80%;\" type=\"button\" onclick=\"window.location.href ='/.'\" value=  Return ></div>";
  server.send(404, "text/plain", message);
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
      return;
    } else {
      inputString += inChar;
    }
  }
  if (stringComplete) {
    if (getUartScopeFlag()) {
      uartIntData = inputString.toInt();
      if (uartIntData) {
        setUartScopeData(inputString);
        Serial.println(inputString);
      } else if (inputString == "0") {
        setUartScopeData(inputString);
        Serial.println(inputString);
      } else {
        String line = "SERIAL UART " + inputString;
        webSocket.broadcastTXT(line);
        Serial.println(line);
      }
    } else {
      String line = "SERIAL UART " + inputString;
      webSocket.broadcastTXT(line);
      Serial.println(line);
    }
    inputString = "";
    stringComplete = false;
  }
}

long max_sketch_size() {
  long ret = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  return ret;
}

void OTASettings() {
    // Upload firmware page
    server.on("/ota", HTTP_GET, []() {
    ///StopPorts();  // DAGNALL, note this is NEEDED in NMEA3 here!, not later on! If not stopped, 
    DoingOTA(true); // stops disconnect!
    String html = ""; 
    //html += FPSTR(Header); 
    html += "&nbsp;</div></div>";
   // html += "<br><center>" + String(soft_version) + "</center>";
    html += FPSTR(OTA_STYLE);
    html += FPSTR(OTA_START);
    //html += OTA_START();
    html += FPSTR(OTA_UPLOAD);
    server.send_P(200, "text/html", html.c_str());
  });
  // Handling uploading firmware file
    server.on("/ota", HTTP_POST, []() { 
    //StopPorts();  // leaving here just in case it did something..
    server.send(200, "text/plain", (Update.hasError()) ? "Update: fail\n" : "Update: OK!\n");
    delay(500);
    DoingOTA(false);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Firmware update initiated: %s\r\n", upload.filename.c_str());  // dagnall should this be serial2 for debugging?-- but the Stop serial that I re-instated stops these being sent!
      //uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      uint32_t maxSketchSpace = max_sketch_size();
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
      // Store the next milestone to output
      uint16_t chunk_size  = 51200;
      static uint32_t next = 51200;
      // Check if we need to output a milestone (100k 200k 300k)
      if (upload.totalSize >= next) {
        Serial.printf("%dk ", next / 1024);    //// dagnall  this be serial2 for debugging?  
        LEDFLASH();  // can add flashing leds here to show progress on leds.. 
        next += chunk_size;
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("\r\nFirmware update successful: %u bytes\r\nRebooting...\r\n", upload.totalSize);      
        } else {
        Update.printError(Serial);
      }
    }
  });
}
