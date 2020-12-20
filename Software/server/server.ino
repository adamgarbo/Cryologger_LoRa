/*
  Cryologger: RFM95W Reliable Datagram Server

  Date: December 13, 2020

  Description:
  - Example sketch showing how to create a simple addressed, reliable messaging
  server with the RHReliableDatagram class, using the RH_RF95 driver to control
  a RF95 radio.
  - Designed to work with the other RF95 reliable datagram client
*/

// ----------------------------------------------------------------------------
// Libraries
// ----------------------------------------------------------------------------
#include <ArduinoLowPower.h>    // https://github.com/arduino-libraries/ArduinoLowPower
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <RTCZero.h>            // https://github.com/arduino-libraries/RTCZero
#include <SdFat.h>              // https://github.com/greiman/SdFat
#include <SPI.h>
#include <TinyGPS++.h>          // https://github.com/mikalhart/TinyGPSPlus
#include <Wire.h>

// ----------------------------------------------------------------------------
// Debugging macros
// ----------------------------------------------------------------------------
#define DEBUG true

#if DEBUG
#define DEBUG_PRINT(x)            Serial.print(x)
#define DEBUG_PRINTLN(x)          Serial.println(x)
#define DEBUG_PRINT_HEX(x)        Serial.print(x, HEX)
#define DEBUG_PRINTLN_HEX(x)      Serial.println(x, HEX)
#define DEBUG_PRINT_DEC(x, y)     Serial.print(x, y)
#define DEBUG_PRINTLN_DEC(x, y)   Serial.println(x, y)
#define DEBUG_WRITE(x)            Serial.write(x)

#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT_HEX(x)
#define DEBUG_PRINTLN_HEX(x)
#define DEBUG_PRINT_DEC(x, y)
#define DEBUG_PRINTLN_DEC(x, y)
#define DEBUG_WRITE(x)
#endif

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------
#define SERVER_ADDRESS 1
#define CLIENT_ADDRESS 2

// ----------------------------------------------------------------------------
// Port definitions
// ----------------------------------------------------------------------------
#define GpsSerial     Serial1

// ----------------------------------------------------------------------------
// RFM95W radio definitions
// ----------------------------------------------------------------------------
#define RF95_FREQ     915.0   // Radio frequency (MHz)
#define RF95_PW       13      // Transmit power (dBm)
#define RF95_SF       7       // Spreading factor
#define RF95_BW       125000  // Bandwidth (MHz)
#define RF95_CR       5       // Coding rate
#define RF95_CRC      true    // Cyclic Redundancy Check (CRC) 

// ----------------------------------------------------------------------------
// Pin definitions
// ----------------------------------------------------------------------------
//#define PIN_RF95_RST  A4
#define PIN_GPS_EN    A5
#define PIN_SD_CS     4
#define PIN_RF95_INT  5
#define PIN_RF95_CS   6
#define PIN_VBAT      9
#define PIN_MISO      22
#define PIN_MOSI      23
#define PIN_SCK       24

#define LED_GREEN     8
#define LED_RED       13
// ----------------------------------------------------------------------------
// Object instantiations
// ----------------------------------------------------------------------------
RTCZero     rtc;
SdFat       sd;
SdFile      file;
TinyGPSPlus gps;

// Singleton instance of the radio driver
RH_RF95 driver(PIN_RF95_CS, PIN_RF95_INT);

// Class to manage message delivery and receipt using declared radio driver
RHReliableDatagram manager(driver, SERVER_ADDRESS);

// ----------------------------------------------------------------------------
// Global variables
// ----------------------------------------------------------------------------
volatile bool alarmFlag       = false;  // RTC alarm ISR flag
volatile bool watchdogFlag    = false;  // Watchdog Timer ISR flag
volatile int  watchdogCounter = 0;      // Watchdog Timer interrupt counter
bool          ledState        = LOW;    // LED toggle flag
bool          rtcSyncFlag     = false;  // RTC synchronization flag
byte          alarmSeconds    = 0;      // Rolling alarm seconds
byte          alarmMinutes    = 1;      // Rolling alarm minutes
byte          alarmHours      = 0;      // Rolling alarm hours
unsigned long previousMillis  = 0;      // Global millis() timer

unsigned long unixtime        = 0;
float         voltage         = 0.0;    //
char          fileName[30]    = "";

// Dont put this on the stack:
// See: https://stackoverflow.com/questions/46437423/how-can-i-avoid-putting-this-variable-on-the-stack
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// Union/structure to store and send data byte-by-byte via LoRa
typedef union {
  struct {
    uint32_t  unixtime;         // UNIX Epoch time                  (4 bytes)
    float     latitude;         // GPS latitude                     (4 bytes)
    float     longitude;        // GPS longitude                    (4 bytes)
    uint32_t  satellites;       // Number of GPS satellites         (4 bytes)
    uint32_t  hdop;             // GPS HDOP                         (4 bytes)
    float     voltage;          // Battery voltage                  (4 bytes)
    int16_t   rssi;             // RSSI of LoRa server transmission (2 bytes)
    int16_t   snr;              // SNR of LoRa server transmission  (2 bytes)
    uint16_t  transmitCounter;  // Message counter                  (2 bytes)
  } __attribute__((packed));
  uint8_t bytes[30]; // Size of structure (30 bytes)
} LoraPacket;

LoraPacket message;

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------
void setup() {

  // Pin assignments
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_GPS_EN, OUTPUT);
  //pinMode(PIN_RF95_RST, OUTPUT);
  pinMode(PIN_VBAT, INPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_GPS_EN, LOW);
  digitalWrite(PIN_SD_CS, HIGH);
  digitalWrite(PIN_RF95_CS, HIGH);
  //digitalWrite(PIN_RF95_RST, HIGH);

  analogReadResolution(12); // Set analog resolution to 12-bits

  Serial.begin(115200);
  //while (!Serial) ; // Wait for serial port to be available
  delay(4000);

  printLine(80);
  DEBUG_PRINTLN("RFM95W reliable datagram server");
  printLine(80);

  Wire.begin(); // Initialize I2C
  SPI.begin();  // Initialize SPI

  configureRtc();   // Configure real-time clock
  syncRtc();        // Sync RTC with GPS
  configureLora();  // Configure RFM95W
  configureSd();    // Configure microSD
  createLogFile();  // Create log file

  DEBUG_PRINTLN("Listening for messages...");

  blinkLed(LED_GREEN, 5, 100);
  blinkLed(LED_RED, 5, 100);

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

      char tempBuffer[100];
      sprintf(tempBuffer, "Request from: 0x%02X Size: %d RSSI: %d SNR: %d",
              from, len, driver.lastRssi(), driver.lastSNR() );
      //DEBUG_PRINTLN(tempBuffer);

      // Write incoming message buffer to union/structure
      for (int i = 0; i < len; ++i) {
        message.bytes[i] = buf[i];
      }

      // Log incoming transmission
      logData();

      // Print union/structure payload contents
      //printUnion(); // 
      printCsv(); //
      //printUnionHex(); // Display data in hexadecimal

      // Send reply back to the originator client
      uint8_t data[] = "OK";
      if (!manager.sendtoWait(data, sizeof(data), from)) {
        DEBUG_PRINTLN("sendtoWait failed");
      }
      blinkLed(LED_GREEN, 2, 100);
    }
  }
}
