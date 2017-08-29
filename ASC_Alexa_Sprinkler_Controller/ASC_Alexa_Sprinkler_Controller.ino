/*
 * ========================================================================================
 * Title: ASC - Alexa Sprinkler Controller
 * Description: ESP8266 Based, Arduino compatible, Alexa commandable, sprinkler controller
 * Author: Ramsey Gill  
 * License MIT
 * Homepage: https://hackaday.io/project/26850-alexa-enabled-sprinkler-controller
 * GitHub: https://github.com/ramseygill/alexa-sprinkler
 * ========================================================================================
 * NOTES, Credit, Sources, and Props
 * http://www.geekstips.com/arduino-time-sync-ntp-server-esp8266-udp/
 * Adafruit-MCP23017-Arduino-Library
 * Copyright (c) 2012, Adafruit Industries All rights reserved.
 * https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library/blob/master/license.txt
 */
 //LIBRARIES-------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "SSD1306.h" // SSD1306
#include "Adafruit_MCP23017.h" //Adafruit-MCP23017-Arduino-Library

 //CONSTANTS/VARIABLES/GLOBALS-------------------------------------------------------------
Adafruit_MCP23017 mcp; //Adafruit-MCP23017-Arduino-Library
SSD1306  display(0x3c, D3, D5); // SSD1306 Pin Assignments
MDNSResponder mdns;

ESP8266WebServer server(80);              //server port
String webPage;
const char* ssid     = "skynet-2GHz";     //wifi SSID
const char* password = "haxor1337";       //wifi password

//PIN DEFINITIONS

//VARIABLES
int relayState = 0;

 //SETUP-----------------------------------------------------------------------------------
void setup() {

  mcp.begin();      // use default address 0 Adafruit-MCP23017-Arduino-Library
  
  display.init(); //SSD12306 init
  
  display.flipScreenVertically(); //SSD12306
  display.setFont(ArialMT_Plain_10); //SSD12306 set font

//set PORT A I/O expander pins to OUTPUTS
  mcp.pinMode(0, OUTPUT); //MCP23017 PIN 21 GPA0 (K1 RELAY)
  mcp.pinMode(1, OUTPUT); //MCP23017 PIN 22 GPA1 (K2 RELAY)
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.pinMode(5, OUTPUT);
  mcp.pinMode(6, OUTPUT);
  mcp.pinMode(7, OUTPUT);

// ensure that each pin defaults to HIGH (relay off)
// these cheap blue relay modules use active low logic.
  mcp.digitalWrite(0, HIGH); //K1
  mcp.digitalWrite(1, HIGH); //K2
  mcp.digitalWrite(2, HIGH); //K3
  mcp.digitalWrite(3, HIGH); //K4
  mcp.digitalWrite(4, HIGH); //K5
  mcp.digitalWrite(5, HIGH); //K6
  mcp.digitalWrite(6, HIGH); //K7
  mcp.digitalWrite(7, HIGH); //K8

//Webpage Heading
  webPage += "<h1>ASC - Alexa Sprinkler Controller</h1>";
//All on objects
  webPage += "<p>ALL RELAYS "; //name of button
  webPage += "<a href=\"allOn\"><button>ON</button></a>&nbsp;"; //on button object
  webPage += "<a href=\"allOff\"><button>OFF</button></a></p>"; //off button object
//Relay 1 button objects
  webPage += "<p>Relay #1 ";
  webPage += "<a href=\"relay1On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay1Off\"><button>OFF</button></a></p>";
//Relay 2 button objects
  webPage += "<p>Relay #2 ";
  webPage += "<a href=\"relay2On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay2Off\"><button>OFF</button></a></p>";
//Relay 3 button objects
  webPage += "<p>Relay #3 ";
  webPage += "<a href=\"relay3On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay3Off\"><button>OFF</button></a></p>";
//Relay 4 button objects
  webPage += "<p>Relay #4 ";
  webPage += "<a href=\"relay4On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay4Off\"><button>OFF</button></a></p>";
//Relay 5 button objects
  webPage += "<p>Relay #5 ";
  webPage += "<a href=\"relay5On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay5Off\"><button>OFF</button></a></p>";
//Relay 6 button objects
  webPage += "<p>Relay #6 ";
  webPage += "<a href=\"relay6On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay6Off\"><button>OFF</button></a></p>";
//Relay 7 button objects
  webPage += "<p>Relay #7 ";
  webPage += "<a href=\"relay7On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay7Off\"><button>OFF</button></a></p>";
//Relay 8 button objects
  webPage += "<p>Relay #8 ";
  webPage += "<a href=\"relay8On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay8Off\"><button>OFF</button></a></p>";

  Serial.begin(115200); //start UART seral communication at 115200 baud
  delay(100);
 
  Serial.println(); 
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) 
    Serial.println("MDNS responder started");
 
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  //RELAY 1 ------------------------------------
  server.on("/relay1On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    mcp.digitalWrite(0, LOW);
    Serial.println("RELAY 1 ON");
    delay(500);
  });
  server.on("/relay1Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    mcp.digitalWrite(0, HIGH);;
    Serial.println("RELAY 1 OFF");
    delay(500); 
  });
  //RELAY 2 ------------------------------------
    server.on("/relay2On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    mcp.digitalWrite(1, LOW);
    Serial.println("RELAY 2 ON");
    delay(500);
  });
  server.on("/relay2Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    mcp.digitalWrite(1, HIGH);
    Serial.println("RELAY 2 OFF");
    delay(500); 
  });
    //RELAY 3 ------------------------------------
    server.on("/relay3On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    mcp.digitalWrite(2, LOW);
    Serial.println("RELAY 3 ON");
    delay(500);
  });
  server.on("/relay3Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    mcp.digitalWrite(2, HIGH);
    Serial.println("RELAY 3 OFF");
    delay(500); 
  });
    //RELAY 4 ------------------------------------
    server.on("/relay4On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    mcp.digitalWrite(3, LOW);
    Serial.println("RELAY 4 ON");
    delay(500);
  });
  server.on("/relay4Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    mcp.digitalWrite(3, HIGH);
    Serial.println("RELAY 4 OFF");
    delay(500); 
  });
    //RELAY 5 ------------------------------------
    server.on("/relay5On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    mcp.digitalWrite(4, LOW);
    Serial.println("RELAY 5 ON");
    delay(500);
  });
  server.on("/relay5Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    mcp.digitalWrite(4, HIGH);
    Serial.println("RELAY 5 OFF");
    delay(500); 
  });
    //RELAY 6 ------------------------------------
    server.on("/relay6On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    mcp.digitalWrite(5, LOW);
    Serial.println("RELAY 6 ON");
    delay(500);
  });
  server.on("/relay6Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    mcp.digitalWrite(5, HIGH);
    Serial.println("RELAY 6 OFF");
    delay(500); 
  });
    //RELAY 7 ------------------------------------
    server.on("/relay7On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    mcp.digitalWrite(6, LOW);
    Serial.println("RELAY 7 ON");
    delay(500);
  });
  server.on("/relay7Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    mcp.digitalWrite(6, HIGH);
    Serial.println("RELAY 7 OFF");
    delay(500); 
  });
    //RELAY 8 ------------------------------------
    server.on("/relay8On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    mcp.digitalWrite(7, LOW);
    Serial.println("RELAY 8 ON");
    delay(500);
  });
  server.on("/relay8Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    mcp.digitalWrite(7, HIGH);
    Serial.println("RELAY 8 OFF");
    delay(500); 
  });
  
      //ALL RELAYS ------------------------------------
    server.on("/allOn", [](){ //ON
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    mcp.digitalWrite(0, LOW); //K1
    mcp.digitalWrite(1, LOW);
    mcp.digitalWrite(2, LOW);
    mcp.digitalWrite(3, LOW);
    mcp.digitalWrite(4, LOW);
    mcp.digitalWrite(5, LOW);
    mcp.digitalWrite(6, LOW);
    mcp.digitalWrite(7, LOW);
    
      Serial.println("ALL ON");

      relayState = 1; //set state for screen update
      
    delay(100);
  });
  
  server.on("/allOff", [](){ //OFF
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    mcp.digitalWrite(0, HIGH);
    mcp.digitalWrite(1, HIGH);
    mcp.digitalWrite(2, HIGH);
    mcp.digitalWrite(3, HIGH);
    mcp.digitalWrite(4, HIGH);
    mcp.digitalWrite(5, HIGH);
    mcp.digitalWrite(6, HIGH);
    mcp.digitalWrite(7, HIGH);
    
      Serial.println("ALL OFF");

      relayState = 0; //set state for screen update
      
    delay(100); 
  });

  server.begin();
  Serial.println("HTTP server started");
}
 
 //MAIN LOOP-------------------------------------------------------------------------------
void loop() {
  display.clear(); //wipe display clean to refresh it
  updateIP(); //function call for display update
  server.handleClient(); //routine for webserver
  delay(50); // loop governor
}

//FUNCTIONS--------------------------------------------------------------------------------
void updateIP(){
  String ipaddress = WiFi.localIP().toString(); // convert IP address array to string
  //display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "IP Address");
  display.drawString(0, 15, (ipaddress));
  
  if(relayState == 1){
    display.drawString(0, 30, "ALL ON");
  }
  else{
    display.drawString(0, 30, "ALL OFF");
}
  display.display(); //write display buffer
}
//----------------------------------------------------------------------------------------
