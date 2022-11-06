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

static const uint8_t LED = 16;
#ifdef ESP32  
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
 #else
  #include <ESP8266WiFi.h>
 // #include <ESP8266WiFiMulti.h>
 // #include <Hash.h>  // used by arduinoota??
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
#endif


#include <WebSocketsServer.h>


#include <WiFiClient.h>
#include <WiFiUdp.h>

#include <DNSServer.h>
#include <WiFiManager.h>  // WIFI Manager by tzapu (tested 2.0.14)
#include <Wire.h>
#include "ScopeCommands.h"
//#include "miniDB.h" // called from scope commands and websocket interprete
#include "webStripChart.h"
#include "WebsocketInterpreter.h"
#include "OTA_Web.h"

byte APMODE_BOOT_PIN = D7;  //DAG  press this pin to ground to start in AP mode..
byte D_in1 = D7;            //DAG
byte D_in2 = D6;            //DAG
String string_in1 = "(D7)";
String string_in2 = "(D6)";
byte _SDA = D4;   //05
byte _SCL = D5;   //04
byte _Data = D2;  // for hx 711
byte _Clock = D1;  // hx711


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


#if defined(ESP8266)
  ESP8266WebServer server(80);
#elif defined(ESP32)
  WebServer server(80);
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
  //ScreenUpdate(5000) ;
  //Serial.print(" Set up screen refresh at "); Serial.println (Screen_U_time());

  if (!digitalRead(APMODE_BOOT_PIN)) {
    WiFi.disconnect();
    WiFi.softAP(ssid, password);
    Serial.println();
    Serial.println("Booting in AP mode");
    Serial.println("Go to 192.168.4.1 to access the oscilloscope");
    Serial.println("NOTE: OTA is NOT available in AP mode");
  } else {
    Serial.println();
    Serial.println("Booting in client mode");
    WiFiManager wifiManager;
    wifiManager.autoConnect(ssid, password);
    Serial.println();
    Serial.print("IP address: ");
    if (!MDNS.begin("oscilloscope")) {
      Serial.println("Error setting up MDNS responder!");
      while (1) {
        delay(1000);
      }
    } else { Serial.println("mDNS responder started");}


    WiFi.softAP(ssid, password);  // and keep standard ap on 192.168.4.1

    Serial.print("Connect to http://oscilloscope.local or http://");
    Serial.println(WiFi.localIP());

    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
  }
  digitalWrite(LED, 1);  //DAG led OFF?
  
  WebserverSetup();

  Wire.begin(_SDA, _SCL);
  scopeInit();
  setWS_Timer(500);  // initial  lazy flash timer for scope sampling rate timebase gets reset when pc connects html
  setsampleuSTimer(5000);  //us = 5ms
  SetScalesConnected(0);
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
  Serial.print("Connect to http://oscilloscope.local or http://");
    Serial.println(WiFi.localIP());
  clearADCScopeData1();
  clearADCScopeData2();
  setPAUSE(false);
  
 // Set_Data_RTS (true);
 // Set_CTS (true);
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
 // ArduinoOTA.handle();
  webSocket.loop();
  server.handleClient();
  //Original
  if ((currentTime - oldTimeADC) >= (getsampleuSTimer()/1000) ) {  //  sample rate is sent in us 
    ADCHandler(); // now handles duplex 
    oldTimeADC = currentTime;
  }

  if (webSocketData != "") {
    webSocketDataInterpreter(webSocket, webSocketData);
   // Serial.println("Websocket data Handle");
    webSocketData = "";
  }

  if ((!PAUSE()) && getDuplexMode() && Data_RTS()  ){
    //  if ( getDuplexMode() && Data_RTS() && Read_CTS() ){
    //Serial.print("/");Serial.print(readNumberofsamplesRead());
    //Serial.println(scopeHandler(webSocket));  // scopehandler returns the sent data 
    LEDFLASH();
    scopeHandler(webSocket);
    webSocketData = "";
    oldTime = currentTime;
    ResetNumberofSamplesread();
    Set_Data_RTS(false); // resets for new samples
    Set_CTS(false);  // wait for webbrowser to ask?
    //delay(50);

  } 


}







void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      webSocket.close();
      WebserverSetup();
      Serial.printf("[%u] ATTEMPTING RESTART OF WEBSERVER!\r\n", num);  // not compatible with  OTA?
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
      hexdump(payload, length);

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
 // ElegantOTA.begin(&server);    // Start ElegantOTA
  // server.on("/", []() {
  //   server.send(200, "text/plain", "Hi! I am ESP8266.");
  // });

  // ElegantOTA.begin(&server);    // Start ElegantOTA  adds /update
  // server.begin();
  // Serial.println("HTTP server started");
  // return;
  server.on("/", handleStrip);
  //server.on("/HOME", handleRoot);
  server.on("/DATA", SendDATA);
  server.on("/data", SendDATA);
  // server.on("/TEST", handleTest);
  server.on("/STRIP", handleStrip);
  server.onNotFound(handleNotFound);
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
