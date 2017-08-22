/*
 * ========================================================================================
 * Title: ASC - Alexa Sprinkler Controller
 * Description: ESP8266 Based, Arduino compatible, Alexa commandable, sprinkler controller
 * Author: Ramsey Gill  
 * License MIT
 * Homepage: https://hackaday.io/project/26850-alexa-enabled-sprinkler-controller
 * ========================================================================================
 * NOTES
 * http://www.geekstips.com/arduino-time-sync-ntp-server-esp8266-udp/
 */
 //LIBRARIES-------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "SSD1306.h" // SSD1306

 //CONSTANTS/VARIABLES/GLOBALS-------------------------------------------------------------
SSD1306  display(0x3c, D3, D5); // SSD1306 Pin Assignments
MDNSResponder mdns;

ESP8266WebServer server(80);              //server port
String webPage;
const char* ssid     = "skynet-2GHz";     //wifi SSID
const char* password = "haxor1337";       //wifi password

//PIN DEFINITIONS
const int k1Pin = 5;  //ESP-12E D1 RELAYS 1-8
const int k2Pin = 4;  //ESP-12E D2
const int k3Pin = 2;  //ESP-12E D4
const int k4Pin = 12; //ESP-12E D6
const int k5Pin = 13; //ESP-12E D7
const int k6Pin = 16; //ESP-12E D8
//const int k7Pin = 0;  //ESP-12E D3
//const int k8Pin = 14; //ESP-12E D5
//Wire SDA on OLED to D3 (GPIO0) pin on ESP-12E (used for SSD1306 OLED)
//Wire SCL on OLED to D5 (GPIO14) pin on ESP-12E (used for SSD1306 OLED)

int relayState = 0;

 //SETUP-----------------------------------------------------------------------------------
void setup() {

  display.init(); //SSD12306 init
  
  display.flipScreenVertically(); //SSD12306
  display.setFont(ArialMT_Plain_10); //SSD12306 set font
  
  pinMode(k1Pin, OUTPUT);  // set relay pins as outputs
  pinMode(k2Pin, OUTPUT);  //
  pinMode(k3Pin, OUTPUT);  //
  pinMode(k4Pin, OUTPUT);  //
  pinMode(k5Pin, OUTPUT);  //
  pinMode(k6Pin, OUTPUT);  //
  //pinMode(k7Pin, OUTPUT);  //
  //pinMode(k8Pin, OUTPUT);  //

  digitalWrite(k1Pin, HIGH); // ensure that each pin defaults to HIGH (relay off)
  digitalWrite(k2Pin, HIGH); // these cheap blue relay modules use inverted logic 
  digitalWrite(k3Pin, HIGH);
  digitalWrite(k4Pin, HIGH);
  digitalWrite(k5Pin, HIGH);
  digitalWrite(k6Pin, HIGH);
  //digitalWrite(k7Pin, HIGH);
  //digitalWrite(k8Pin, HIGH);

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
/*
//Relay 7 button objects
  webPage += "<p>Relay #7 ";
  webPage += "<a href=\"relay7On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay7Off\"><button>OFF</button></a></p>";
//Relay 8 button objects
  webPage += "<p>Relay #8 ";
  webPage += "<a href=\"relay8On\"><button>ON</button></a>&nbsp;";
  webPage += "<a href=\"relay8Off\"><button>OFF</button></a></p>";
 */
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
    digitalWrite(k1Pin, LOW);
    Serial.println("RELAY 1 ON");
    delay(500);
  });
  server.on("/relay1Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    digitalWrite(k1Pin, HIGH);
    Serial.println("RELAY 1 OFF");
    delay(500); 
  });
  //RELAY 2 ------------------------------------
    server.on("/relay2On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    digitalWrite(k2Pin, LOW);
    Serial.println("RELAY 2 ON");
    delay(500);
  });
  server.on("/relay2Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    digitalWrite(k2Pin, HIGH);
    Serial.println("RELAY 2 OFF");
    delay(500); 
  });
    //RELAY 3 ------------------------------------
    server.on("/relay3On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    digitalWrite(k3Pin, LOW);
    Serial.println("RELAY 3 ON");
    delay(500);
  });
  server.on("/relay3Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    digitalWrite(k3Pin, HIGH);
    Serial.println("RELAY 3 OFF");
    delay(500); 
  });
    //RELAY 4 ------------------------------------
    server.on("/relay4On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    digitalWrite(k4Pin, LOW);
    Serial.println("RELAY 4 ON");
    delay(500);
  });
  server.on("/relay4Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    digitalWrite(k4Pin, HIGH);
    Serial.println("RELAY 4 OFF");
    delay(500); 
  });
    //RELAY 5 ------------------------------------
    server.on("/relay5On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    digitalWrite(k5Pin, LOW);
    Serial.println("RELAY 5 ON");
    delay(500);
  });
  server.on("/relay5Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    digitalWrite(k5Pin, HIGH);
    Serial.println("RELAY 5 OFF");
    delay(500); 
  });
    //RELAY 6 ------------------------------------
    server.on("/relay6On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    digitalWrite(k6Pin, LOW);
    Serial.println("RELAY 6 ON");
    delay(500);
  });
  server.on("/relay6Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    digitalWrite(k6Pin, HIGH);
    Serial.println("RELAY 6 OFF");
    delay(500); 
  });
   /*
    //RELAY 7 ------------------------------------
    server.on("/relay7On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    //digitalWrite(k7Pin, LOW);
    Serial.println("RELAY 7 ON");
    delay(500);
  });
  server.on("/relay7Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    //digitalWrite(k7Pin, HIGH);
    Serial.println("RELAY 7 OFF");
    delay(500); 
  });
    //RELAY 8 ------------------------------------
    server.on("/relay8On", [](){
    server.send(200, "text/html", webPage);
    // Turn on RELAY
    //digitalWrite(k8Pin, LOW);
    Serial.println("RELAY 8 ON");
    delay(500);
  });
  server.on("/relay8Off", [](){
    server.send(200, "text/html", webPage);
    //Turn off RELAY
    //digitalWrite(k8Pin, HIGH);
    Serial.println("RELAY 8 OFF");
    delay(500); 
  });
  */
      //ALL RELAYS ------------------------------------
    server.on("/allOn", [](){ //ON
    server.send(200, "text/html", webPage);
    // Turn on RELAY
      digitalWrite(k1Pin, LOW);
      digitalWrite(k2Pin, LOW);
      digitalWrite(k3Pin, LOW);
      digitalWrite(k4Pin, LOW);
      digitalWrite(k5Pin, LOW);
      digitalWrite(k6Pin, LOW);
      //digitalWrite(k7Pin, LOW);
      //digitalWrite(k8Pin, LOW);
      Serial.println("ALL ON");

      relayState = 1; //set state for screen update
      
    delay(100);
  });
  server.on("/allOff", [](){ //OFF
    server.send(200, "text/html", webPage);
    //Turn off RELAY
      digitalWrite(k1Pin, HIGH);
      digitalWrite(k2Pin, HIGH);
      digitalWrite(k3Pin, HIGH);
      digitalWrite(k4Pin, HIGH);
      digitalWrite(k5Pin, HIGH);
      digitalWrite(k6Pin, HIGH);
      //digitalWrite(k7Pin, HIGH);
      //digitalWrite(k8Pin, HIGH);
      Serial.println("ALL OFF");

      relayState = 0; //set state for screen update
      
    delay(100); 
  });

  server.begin();
  Serial.println("HTTP server started");
}
 
 //MAIN LOOP-------------------------------------------------------------------------------
void loop() {
  display.clear(); //wipe display our to refresh it
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
