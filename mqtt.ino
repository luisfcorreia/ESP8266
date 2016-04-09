/* NodeMCU v1.0 SPI Pinout

 HCS   -> GPIO15 - D8
 HMOSI -> GPIO13 - D7
 HMISO -> GPIO12 - D6
 HSCLK -> GPIO14 - D5

*/

/* MCP3208 Pinout:
        ___
 CH0 1 | u | 16 VCC
 CH1 2 |   | 15 VREF
 CH2 3 |   | 14 AGND
 CH3 4 |   | 13 CLK
 CH4 5 |   | 12 MISO
 CH5 6 |   | 11 MOSI
 CH6 7 |   | 10 CS
 CH7 8 |___| 9  DGND

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************** SPI Pinout Setup *********************************/

#define SELPIN D8    //CS
#define DATAOUT D7   //MOSI
#define DATAIN D6    //MISO
#define SPICLOCK D5  //SCLK

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "...your SSID..."
#define WLAN_PASS       "...your password..."

/************************* MQTT Server Setup *********************************/

#define MQTT_SERVER_HOST      "YOUR_MQTT_SERVER_HOST"
#define MQTT_SERVERPORT  1883                   // use 8883 for SSL
#define MQTT_USER        "...your MQTT username..."
#define MQTT_KEY         "...your MQTT key..."

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = MQTT_SERVER_HOST;
const char MQTT_USERNAME[] PROGMEM  = MQTT_USER;
const char MQTT_PASSWORD[] PROGMEM  = MQTT_KEY;

long adctime;
long mqtttime;
long adcdelay = 1000;
long mqttdelay = 5000;


// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for MQTT follow the form: <username>/feeds/<feedname>
const char CURRENT_FEED[] PROGMEM = "/feeds/current";
Adafruit_MQTT_Publish current = Adafruit_MQTT_Publish(&mqtt, CURRENT_FEED);

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();


void setup_pins() {
  //set pin modes
  pinMode(SELPIN, OUTPUT);
  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK, OUTPUT);
  //disable device to start with
  digitalWrite(SELPIN,HIGH);
  digitalWrite(DATAOUT,LOW);
  digitalWrite(SPICLOCK,LOW);
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

int read_adc(int channel){
  int adcvalue = 0;
  byte commandbits = B11000000; //command bits - start, mode, chn (3), dont care (3)

  //allow channel selection
  commandbits|=((channel-1)<<3);

  digitalWrite(SELPIN,LOW); //Select adc
  // setup bits to be written
  for (int i=7; i>=3; i--){
    digitalWrite(DATAOUT,commandbits&1<<i);
    //cycle clock
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }

  digitalWrite(SPICLOCK,HIGH);    //ignores 2 null bits
  digitalWrite(SPICLOCK,LOW);
  digitalWrite(SPICLOCK,HIGH);
  digitalWrite(SPICLOCK,LOW);

  //read bits from adc
  for (int i=11; i>=0; i--){
    adcvalue+=digitalRead(DATAIN)<<i;
    //cycle clock
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }
  digitalWrite(SELPIN, HIGH); //turn off device
  return adcvalue;
}

float readvalue = 0;

// set next milles serial port should be read
void setAdcMillies() {
  adctime = millis() + adcdelay;
}

// set next milles serial port should be read
void setMQTTMillies() {
  mqtttime = millis() + mqttdelay;
}


void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Current Node via MQTT"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // Set delays
  setAdcMillies();
  setMQTTMillies();
}



void loop() {

  // read ADC each adctime
  if (millis() >= adctime) {
	readvalue = read_adc(1)    
	Serial.print(F("\nI've read this ->"));
	Serial.print(readvalue);
	Serial.print("...");
	
	setAdcMillies();
  }

  // publish to MQTT each mqtttime
  if (millis() >= mqtttime) {
	  if (! current.publish(readvalue)) {
		Serial.println(F("MQTT send Failed"));
	  } else {
		Serial.println(F("MQTT send OK!"));
	  }
	
	setMQTTMillies();
  }

}
