
/***************************************************
  This is an example for the Adafruit CC3000 Wifi Breakout & Shield

  Designed specifically to work with the Adafruit WiFi products:
  ----> https://www.adafruit.com/products/1469

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried, Kevin Townsend and Phil Burgess for
  Adafruit Industries.  BSD license, all text above must be included
  in any redistribution
 ****************************************************/

/*
This example queries an NTP time server to get the current "UNIX time"
(seconds since 1/1/1970, UTC (GMT)), then uses the Arduino's internal
timer to keep relative time.  The clock is re-synchronized roughly
once per day.  This minimizes NTP server misuse/abuse.

The RTClib library (a separate download, and not used here) contains
functions to convert UNIX time to other formats if needed.
*/

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <RTClib.h>
#include "sha256.h"
#include <Wire.h>

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed but DI

#define WLAN_SSID       "Bits"        // cannot be longer than 32 characters!
#define WLAN_PASS       "xxx"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

Adafruit_CC3000_Client client;

const unsigned long
  connectTimeout  = 15L * 1000L, // Max time to wait for server connection
  responseTimeout = 15L * 1000L; // Max time to wait for data from server
int
  countdown       = 0;  // loop() iterations until next time server query
unsigned long
  lastPolledTime  = 0L, // Last value retrieved from time server
  currentTime     = 0L, // currentTime
  sketchTime      = 0L; // CPU milliseconds since last server query

 
#define WEBSITE      "192.168.0.33"
#define WEBPAGE      "/"
#define IDLE_TIMEOUT_MS  3000
uint32_t ip;

int tempSensorPin = 0;

void setup(void)
{
  Serial.begin(115200);
  Serial.println(F("Hello, CC3000!\n")); 
  
  printFreeRam();

  displayDriverMode();
  
  Serial.println(F("\nInitialising the CC3000 ..."));
  if (!cc3000.begin()) {
    Serial.println(F("Unable to initialise the CC3000! Check your wiring?"));
    for(;;);
  }

  uint16_t firmware = checkFirmwareVersion();
  if (firmware < 0x113) {
    Serial.println(F("Wrong firmware version!"));
    for(;;);
  } 
  
  displayMACAddress();
  
  Serial.println(F("\nDeleting old connection profiles"));
  if (!cc3000.deleteProfiles()) {
    Serial.println(F("Failed!"));
    while(1);
  }

  /* Attempt to connect to an access point */
  char *ssid = WLAN_SSID;             /* Max 32 chars */
  Serial.print(F("\nAttempting to connect to ")); Serial.println(ssid);
  
  /* NOTE: Secure connections are not available in 'Tiny' mode! */
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) {
    delay(100); // ToDo: Insert a DHCP timeout!
  }

  /* Display the IP address DNS, Gateway, etc. */  
  while (!displayConnectionDetails()) {
    delay(1000);
  }
}


// To reduce load on NTP servers, time is polled once per roughly 24 hour period.
// Otherwise use millis() to estimate time since last query.  Plenty accurate.
void loop(void) {
  
  printFreeRam();
  
  updateCurrentUnixTimeStamp(currentTime, countdown, sketchTime, lastPolledTime);
  
  //currentTime = 1399836892;
  
  Serial.print(F("Current UNIX time: "));
  Serial.println(currentTime);


  int reading = analogRead(tempSensorPin);  
  float voltage = reading * 5.0;
  voltage /= 1024.0; 
  Serial.print(voltage); Serial.println(" volts");

 // now print out the temperature
 float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((voltage - 500mV) times 100)
 Serial.print(temperatureC); Serial.println(" degrees C");
 
 // now convert to Fahrenheit
 float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
 Serial.print(temperatureF); Serial.println(" degrees F");

  reportReadingsToServer(cc3000, currentTime, temperatureF);


  printFreeRam();

  //dynamoDBWrite(TABLE_NAME, currentTime, currentTime, 4.20f);

  Serial.println(F("Sleeping..."));
  delay(30000L); // Pause 15 seconds
}

void printFreeRam() {
  Serial.print(F("Free Memory: "));
  Serial.println(freeRam());
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
