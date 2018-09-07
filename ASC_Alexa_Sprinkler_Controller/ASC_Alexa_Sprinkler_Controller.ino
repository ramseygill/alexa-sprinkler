/*
   ===============================================================================================
   Title: ASC - Alexa Sprinkler Controller
   Description: ESP8266 Based, Arduino compatible, Alexa commandable, sprinkler controller.
   Author: Ramsey Gill
   License MIT
   Homepage: https://hackaday.io/project/26850-alexa-enabled-sprinkler-controller
   GitHub: https://github.com/ramseygill/alexa-sprinkler
   ===============================================================================================
   NOTES, Credit, Sources, and Props
   NTP Library:https://github.com/arduino-libraries/NTPClient
   Adafruit-MCP23017-Arduino-Library
   Copyright (c) 2012, Adafruit Industries All rights reserved.
   https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library/blob/master/license.txt
*/
//------------------------------------------------------------------------------------------------
//LIBRARIES
//------------------------------------------------------------------------------------------------

#include <ESP8266WiFi.h>        // ESP8266 (ESP-12E) wifi library
#include "SSD1306.h"            // SSD1306 OLED display 
#include "Adafruit_MCP23017.h"  // Adafruit-MCP23017-Arduino-Library, GPIO EXPANDER
#include "fauxmoESP.h"          // Alexa suppport. (fake WeMo devices)
#include <NTPClient.h>          // Supports NTP server time queries. 
#include <WiFiUdp.h>            // Adds UDP protocol support, required by NTP service.

//------------------------------------------------------------------------------------------------
//VARIABLES AND GLOBALS
//------------------------------------------------------------------------------------------------

// PIN DEFINITION CONSTANTS
const int I2C_SDA = 4;      //I2C bus data pin (SCL). ESP-12E "D1"
const int I2C_SCL = 5;     //I2C bus clock pin (SCL). ESP-12E "D2"
const int ONE_WIRE = 14;     //1 WIRE Interface for AM2301 "HT21" Temp/Humidity. ESP-12E "D5"
const int relayPin_K1 = 0;  //K1 relay control pin. MCP23017 Pin 21/GPA0
const int relayPin_K2 = 1;  //K2 relay control pin. MCP23017 Pin 22/GPA1
const int relayPin_K3 = 2;  //K3 relay control pin. MCP23017 Pin 23/GPA2
const int relayPin_K4 = 3;  //K4 relay control pin. MCP23017 Pin 24/GPA3
const int relayPin_K5 = 4;  //K5 relay control pin. MCP23017 Pin 25/GPA4
const int relayPin_K6 = 5;  //K6 relay control pin. MCP23017 Pin 26/GPA5
const int relayPin_K7 = 6;  //K7 relay control pin. MCP23017 Pin 27/GPA6
const int relayPin_K8 = 7;  //K8 relay control pin. MCP23017 Pin 28/GPA7

// GLOBAL CONSTANTS
const char *ssid     = "skynet-2GHz";   // WIFI network you want to connect to.
const char *password = "haxor1337";     // WIFI network password.
const int baudRate = 115200;            // USB serial baud rate. (DEBUG)

// LIBRARY INITIALIZATION
SSD1306  display(0x3c, I2C_SDA, I2C_SCL); // SSD1306 Pin Assignments
fauxmoESP fauxmo;                         // Initialize fauxmo
Adafruit_MCP23017 mcp;                    // Adafruit-MCP23017-Arduino-Library
WiFiUDP ntpUDP;                           // Initalize WifiUDP library

// By default 'time.nist.gov' is used with 60 seconds update interval and no offset
// Seattle is UTC-7 or -25200 seconds.
// Update interval is one minute or 60000 miliseconds.
// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "0.us.pool.ntp.org", -25200, 60000);

//------------------------------------------------------------------------------------------------
//SETUP
//------------------------------------------------------------------------------------------------
void setup() {

  //START I2C COMMUNICATION
  display.init();   // SSD12306 - OLED screen start I2C comms
  Serial.println("SSD1306 INIT");
  mcp.begin();      // MCP23017 - I/O Expander start I2C comms

  //Flip Screen orientation in software, (pins on top)
  display.flipScreenVertically(); //SSD12306

  //CONFIGURE PIN USAGE
 
  //set PORT A I/O expander pins to OUTPUTS for relay use
  mcp.pinMode(relayPin_K1, OUTPUT);
  mcp.pinMode(relayPin_K2, OUTPUT);
  mcp.pinMode(relayPin_K3, OUTPUT);
  mcp.pinMode(relayPin_K4, OUTPUT);
  mcp.pinMode(relayPin_K5, OUTPUT);
  mcp.pinMode(relayPin_K6, OUTPUT);
  mcp.pinMode(relayPin_K7, OUTPUT);
  mcp.pinMode(relayPin_K8, OUTPUT);

  //SET PIN INITIAL STATE
  //Relay controller uses active low logic, setting each pin to HIGH on boot ensure that they remain off.
  mcp.digitalWrite(relayPin_K1, HIGH);
  mcp.digitalWrite(relayPin_K2, HIGH);
  mcp.digitalWrite(relayPin_K3, HIGH);
  mcp.digitalWrite(relayPin_K4, HIGH);
  mcp.digitalWrite(relayPin_K5, HIGH);
  mcp.digitalWrite(relayPin_K6, HIGH);
  mcp.digitalWrite(relayPin_K7, HIGH);
  mcp.digitalWrite(relayPin_K8, HIGH);

  // FAUXMO DEVICES
  // Each of these devices will appear in Alexa App.
  // Create a group from these devices "Front Yeard Sprinklers"
  // Each controller will have its own letter "A" followed by a zone "1"
  fauxmo.addDevice("Zone A1");
  fauxmo.addDevice("Zone A2");
  fauxmo.addDevice("Zone A3");
  fauxmo.addDevice("Zone A4");
  fauxmo.addDevice("Zone A5");
  fauxmo.addDevice("Zone A6");
  fauxmo.addDevice("Zone A7");
  fauxmo.addDevice("Zone A8");
  fauxmo.onMessage(callback); //function call for fauxmo state check.


  //------------AM2301 Setup----------------------------------------------------------

  //----------------------------------------------------------------------------------

  //------------USB Serial Comms Setup -----------------------------------------------
  Serial.begin(baudRate);                            //start UART serial comm at global deff baudrate
  Serial.println();                                  // add two blank lines
  Serial.println();
  Serial.println("ALEXA Sprinkler Controller V1.0"); //vanity title for serial window
  //----------------------------------------------------------------------------------

  //------------WiFi Setup------------------------------------------------------------
  wifiSetup();     // Function call to kick off WiFi setup.
  //----------------------------------------------------------------------------------
}

//------------------------------------------------------------------------------------------------
//FUNCTIONS (SUBROUTINES)
//------------------------------------------------------------------------------------------------

//UPDATE IP FUNCTION
//------------------------------------------------------------------------------------------------
void updateIP() {
  String ipaddress = WiFi.localIP().toString(); // convert IP address array to string
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "IP:");
  display.drawString(18, 0, (ipaddress));
}
//UPDATE TIME FUNCTION
//------------------------------------------------------------------------------------------------

void updateTime() {
  timeClient.update(); // Requests time update from NTP server
  String timeString = timeClient.getFormattedTime(); // convert time to string
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 16, "TIME:");
  display.drawString(45, 16, (timeString));
}

//FAUXMO CALLBACK FUNCTION add devices here
//------------------------------------------------------------------------------------------------
void callback(uint8_t device_id, const char * device_name, bool state) {
  Serial.printf("[MAIN] %s state: %s\n", device_name, state ? "ON" : "OFF");


  //ZONE 1
  if ( (strcmp(device_name, "Zone A1") == 0) ) {
    if (state) {
      mcp.digitalWrite(relayPin_K1, LOW); //TURN ON RELAY
    } else {
      mcp.digitalWrite(relayPin_K1, HIGH); //TURN OFF RELAY
    }
  }
  //ZONE 2
  if ( (strcmp(device_name, "Zone A2") == 0) ) {
    if (state) {
      mcp.digitalWrite(relayPin_K2, LOW); //TURN ON RELAY
    } else {
      mcp.digitalWrite(relayPin_K2, HIGH); //TURN OFF RELAY
    }
  }
  //ZONE 3
  if ( (strcmp(device_name, "Zone A3") == 0) ) {
    if (state) {
      mcp.digitalWrite(relayPin_K3, LOW); //TURN ON RELAY
    } else {
      mcp.digitalWrite(relayPin_K3, HIGH); //TURN OFF RELAY
    }
  }
  //ZONE 4
  if ( (strcmp(device_name, "Zone A4") == 0) ) {
    if (state) {
      mcp.digitalWrite(relayPin_K4, LOW); //TURN ON RELAY
    } else {
      mcp.digitalWrite(relayPin_K4, HIGH); //TURN OFF RELAY
    }
  }
  //ZONE 5
  if ( (strcmp(device_name, "Zone A5") == 0) ) {
    if (state) {
      mcp.digitalWrite(relayPin_K5, LOW); //TURN ON RELAY
    } else {
      mcp.digitalWrite(relayPin_K5, HIGH); //TURN OFF RELAY
    }
  }
  //ZONE 6
  if ( (strcmp(device_name, "Zone A6") == 0) ) {
    if (state) {
      mcp.digitalWrite(relayPin_K6, LOW); //TURN ON RELAY
    } else {
      mcp.digitalWrite(relayPin_K6, HIGH); //TURN OFF RELAY
    }
  }
  //ZONE 7
  if ( (strcmp(device_name, "Zone A7") == 0) ) {
    if (state) {
      mcp.digitalWrite(relayPin_K7, LOW); //TURN ON RELAY
    } else {
      mcp.digitalWrite(relayPin_K7, HIGH); //TURN OFF RELAY
    }
  }
  //ZONE 8
  if ( (strcmp(device_name, "Zone A8") == 0) ) {
    if (state) {
      mcp.digitalWrite(relayPin_K8, LOW); //TURN ON RELAY
    } else {
      mcp.digitalWrite(relayPin_K8, HIGH); //TURN OFF RELAY
    }
  }
} // end function

// WIFI setup function
//------------------------------------------------------------------------------------------------

void wifiSetup() {
  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", ssid);
  WiFi.begin(ssid, password);

  // Wait
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  timeClient.begin(); // start NTP time client

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

//================================================================================================
//MAIN LOOP
//================================================================================================
void loop() {
  display.clear();     // wipe display clean to refresh it
  updateIP();          // function call for display update
  //updateTemp();        // fuction call for TMP102 temp
  updateTime();
  fauxmo.handle();     // call fauxmo device function
  display.display();   // write display buffer
  Serial.println("LOOP");
  delay(100);           // loop governor, determine how fast this loop runs
}
//================================================================================================

