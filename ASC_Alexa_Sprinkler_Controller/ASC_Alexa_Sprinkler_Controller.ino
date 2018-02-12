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
#include "SparkFunTMP102.h"     // TMP102 I2C temperature sensor 
#include "fauxmoESP.h"          // Alexa suppport. (fake WeMo devices)
#include <NTPClient.h>          // Supports NTP server time queries. 
#include <WiFiUdp.h>            // Adds UDP protocol support, required by NTP service.

//------------------------------------------------------------------------------------------------
//VARIABLES AND GLOBALS
//------------------------------------------------------------------------------------------------

// PIN DEFINITION CONSTANTS
const int I2C_SDA = 0;      //I2C bus data pin (SCL). ESP-12E "D3" GPIO0
const int I2C_SCL = 14;     //I2C bus clock pin (SCL). ESP-12E "D5" GPIO14
const int alertPin = 5;     //TMP102 alert pin. ESP8266 "D1" GPIO5
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
TMP102 sensor0(0x48);                     // Initialize sensor at I2C address 0x48
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
  mcp.begin();      // MCP23017 - I/O Expander start I2C comms
  sensor0.begin();  // TMP102 - Temp Sensor start I2C comms

  //Flip Screen orientation in software, (pins on top)
  display.flipScreenVertically(); //SSD12306

  //CONFIGURE PIN USAGE
  pinMode(alertPin, INPUT); // Declare alertPin as an input TMP102
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


  //------------TMP102 Setup----------------------------------------------------------
  // Initialize sensor0 settings
  // These settings are saved in the sensor, even if it loses power

  // set the number of consecutive faults before triggering alarm.
  // 0-3: 0:1 fault, 1:2 faults, 2:4 faults, 3:6 faults.
  sensor0.setFault(3);  // Trigger alarm immediately

  // set the polarity of the Alarm. (0:Active LOW, 1:Active HIGH).
  sensor0.setAlertPolarity(1); // Active HIGH

  // set the sensor in Comparator Mode (0) or Interrupt Mode (1).
  sensor0.setAlertMode(0); // Comparator Mode.

  // set the Conversion Rate (how quickly the sensor gets a new reading)
  //0-3: 0:0.25Hz, 1:1Hz, 2:4Hz, 3:8Hz
  sensor0.setConversionRate(2);

  //set Extended Mode.
  //0:12-bit Temperature(-55C to +128C) 1:13-bit Temperature(-55C to +150C)
  sensor0.setExtendedMode(0);

  //set T_HIGH, the upper limit to trigger the alert on
  sensor0.setHighTempF(77.0);  // set T_HIGH in F
  //sensor0.setHighTempC(29.4); // set T_HIGH in C

  //set T_LOW, the lower limit to shut turn off the alert
  sensor0.setLowTempF(76.0);  // set T_LOW in F
  //sensor0.setLowTempC(26.67); // set T_LOW in C
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
//UPDATE TEMP FUNCTION (SHITSHOW CODE, BUT IT WORKS)
//------------------------------------------------------------------------------------------------

void updateTemp() {

  float temperature;
  String stringVal = "";
  boolean alertPinState, alertRegisterState;

  // read temperature data and write to OLED screen
  temperature = sensor0.readTempF();
  //temperature = sensor0.readTempC();

  // convert temperature float to string
  stringVal += String(int(temperature)) + "." + String(getDecimal(temperature)); //combining both whole and decimal part in string with a fullstop between them
  //Serial.print("stringVal: ");Serial.println(stringVal);              //display string value

  char charVal[stringVal.length() + 1];                    //initialise character array to store the values
  stringVal.toCharArray(charVal, stringVal.length() + 1);  //passing the value of the string to the character array

  //Serial.print("charVal: ");
  for (uint8_t i = 0; i < sizeof(charVal); i++) //Serial.print(charVal[i]); //display character array

    display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 32, "TEMP:");
  display.drawString(80, 32, "F");
  display.drawString(48, 32, (stringVal));

  // Check for Alert
  alertPinState = digitalRead(alertPin); // read the Alert from pin
  alertRegisterState = sensor0.alert();   // read the Alert from register

  /*
    // Print temperature and alarm state
    Serial.print("Temperature: ");
    Serial.print(temperature);

    Serial.print("\tAlert Pin: ");
    Serial.print(alertPinState);

    Serial.print("\tAlert Register:  ");
    Serial.println(alertRegisterState);
  */
}
//function to extract decimal part of float (SUB ROUTINE OF TEMP UPDATE)
long getDecimal(float val)
{
  int intPart = int(val);
  long decPart = 10 * (val - intPart); //I am multiplying by 1000 assuming that the foat values will have a maximum of 3 decimal places.
  //Change to match the number of decimal places you need
  if (decPart > 0)return (decPart);       //return the decimal part of float number if it is available
  else if (decPart < 0)return ((-1) * decPart); //if negative, multiply by -1
  else if (decPart = 0)return (00);       //return 0 if decimal part of float number is not available
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
  updateTemp();        // fuction call for TMP102 temp
  updateTime();
  fauxmo.handle();     // call fauxmo device function
  display.display();   // write display buffer
  delay(10);           // loop governor, determine how fast this loop runs
}
//================================================================================================

