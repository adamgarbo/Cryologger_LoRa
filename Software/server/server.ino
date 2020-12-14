/*
  Cryologger: RFM95W Reliable Datagram Server

  Date: December 13, 2020

  Description:
  - Example sketch showing how to create a simple addressed, reliable messaging server
  with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
  - It is designed to work with the other RF95 reliable datagram client
*/

// ----------------------------------------------------------------------------
// Libraries
// ----------------------------------------------------------------------------
#include <ArduinoLowPower.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <RTCZero.h>
#include <SdFat.h>
#include <SPI.h>
#include <Wire.h>

// ----------------------------------------------------------------------------
// Debugging macros
// ----------------------------------------------------------------------------
#define DEBUG true

#if DEBUG
#define DEBUG_PRINT(x)        Serial.print(x)
#define DEBUG_PRINTLN(x)      Serial.println(x)
#define DEBUG_PRINT_HEX(x)    Serial.print(x, HEX)
#define DEBUG_PRINTLN_HEX(x)  Serial.println(x, HEX)
#define DEBUG_PRINT_DEC(x)    Serial.print(x, DEC)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT_HEX(x)
#define DEBUG_PRINTLN_HEX(x)
#define DEBUG_PRINT_DEC(x)
#endif

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------
#define SERVER_ADDRESS 1
#define CLIENT_ADDRESS 2


// ----------------------------------------------------------------------------
// RFM95W radio definitions
// ----------------------------------------------------------------------------
#define RF95_FREQ     915.0     // Radio frequency (MHz)
#define RF95_PW       13        // Transmit power (dBm)
#define RF95_SF       7         // Spreading factor
#define RF95_BW       125000    // Bandwidth (MHz)
#define RF95_CR       5         // Coding rate
#define RF95_CRC      true      // Cyclic Redundancy Check (CRC) 

// Moteino M0
#define PIN_RF95_CS   A2
#define PIN_RF95_INT  9

// ----------------------------------------------------------------------------
// Pin definitions
// ----------------------------------------------------------------------------
#define PIN_VBAT        A5
#define PIN_MICROSD_CS  10
#define PIN_MICROSD_EN  11

// ----------------------------------------------------------------------------
// Object instantiations
// ----------------------------------------------------------------------------

RTCZero rtc;
SdFat   sd;    // File system object
SdFile  file;  // Log file

// Singleton instance of the radio driver
RH_RF95 driver(PIN_RF95_CS, PIN_RF95_INT);

// Class to manage message delivery and receipt using driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

// ----------------------------------------------------------------------------
// Global variables
// ----------------------------------------------------------------------------
volatile bool alarmFlag       = false;  // RTC alarm ISR flag
volatile bool watchdogFlag    = false;  // Watchdog Timer ISR flag
volatile int  watchdogCounter = 0;      // Watchdog Timer interrupt counter
bool          ledState        = LOW;    // LED toggle flag
byte          alarmSeconds    = 0;      // Rolling alarm seconds
byte          alarmMinutes    = 1;      // Rolling alarm minutes
byte          alarmHours      = 0;      // Rolling alarm hours
unsigned long previousMillis  = 0;      // Global millis() timer

char          fileName[30]    = "";
char          outputData[100];          // Recording to SD in 512-byte chunks
char          tempData[50];             // Temporary SD data buffer


// Dont put this on the stack:
// See: https://stackoverflow.com/questions/46437423/how-can-i-avoid-putting-this-variable-on-the-stack
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------
void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  //while (!Serial) ; // Wait for serial port to be available
  delay(4000);

  DEBUG_PRINTLN("RFM95W reliable datagram server");
  printDateTime();

  Wire.begin(); // Initialize I2C
  SPI.begin();  // Initialize SPI

  configureRtc();   // Configure real-time clock
  configureLora();  // Configure RFM95W
  //configureSd();    // Configure microSD
  //createLogFile();  // Create log file

  DEBUG_PRINTLN("Listening for messages...");
}

// ----------------------------------------------------------------------------
// Loop
// ----------------------------------------------------------------------------
void loop() {

  if (manager.available()) {

    // Wait for message addressed to server from a client
    uint8_t len = sizeof(buf);
    uint8_t from;

    if (manager.recvfromAck(buf, &len, &from)) {

      char clientBuffer[100];
      sprintf(clientBuffer, "Request from: 0x%02X Size: %d Payload: %s RSSI: %d SNR: %d",
              from, len, buf, driver.lastRssi(), driver.lastSNR() );
      DEBUG_PRINTLN(clientBuffer);

      // Print payload in hexadecimal
      DEBUG_PRINT(" Raw payload: ");
      for (int i = 0; i < len; ++i) {
        DEBUG_PRINT_HEX(buf[i]);
        DEBUG_PRINT(" ");
      }
      DEBUG_PRINTLN();

      uint8_t data[] = "OK";

      // Send a reply back to the originator client
      if (!manager.sendtoWait(data, sizeof(data), from)) {
        DEBUG_PRINTLN("sendtoWait failed");
      }
      blinkLed(2, 25);
    }
  }
}
