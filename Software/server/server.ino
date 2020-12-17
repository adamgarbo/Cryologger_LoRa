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
#include <ArduinoLowPower.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <RTCZero.h>
#include <SdFat.h>
#include <SPI.h>
#include <SPIFlash.h>   // https://github.com/lowpowerlab/spiflash
#include <TinyGPS++.h>  // https://github.com/mikalhart/TinyGPSPlus
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
#define DEBUG_WRITE(x)        Serial.write(x)

#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT_HEX(x)
#define DEBUG_PRINTLN_HEX(x)
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
#define PIN_RF95_CS   A2
#define PIN_VBAT      A5
#define PIN_FLASH_CS  8
#define PIN_RF95_INT  9
#define PIN_SD_CS     10
#define PIN_SD_EN     11
#define PIN_MOSI      19
#define PIN_SCK       20
#define PIN_MISO      21
#define PIN_GPS_EN    4

// ----------------------------------------------------------------------------
// Object instantiations
// ----------------------------------------------------------------------------
RTCZero     rtc;
SdFat       sd;
SdFile      file;
SPIFlash    flash(SS_FLASHMEM);
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

char          fileName[30]    = "";
char          outputData[100];          // Recording to SD in 512-byte chunks
char          tempData[50];             // Temporary SD data buffer

// Dont put this on the stack:
// See: https://stackoverflow.com/questions/46437423/how-can-i-avoid-putting-this-variable-on-the-stack
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// Union/structure to store and send data byte-by-byte via LoRa
typedef union {
  struct {
    uint32_t  unixtime;         // UNIX Epoch time  (4 bytes)
    float     latitude;         // GPS latitude     (4 bytes)
    float     longitude;        // GPS longitude    (4 bytes)
    float     voltage;          // Battery voltage  (4 bytes)
    uint16_t  transmitCounter;  // Message counter  (2 bytes)
  } __attribute__((packed));
  uint8_t bytes[18]; // Size of structure (18 bytes)
} LoraPacket;

LoraPacket message;

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------
void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_GPS_EN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_GPS_EN, LOW);

  Serial.begin(115200);
  //while (!Serial) ; // Wait for serial port to be available
  delay(4000);

  Wire.begin(); // Initialize I2C
  SPI.begin(); // Initialize SPI

  configureFlash(); // Configure Flash
  configureRtc();   // Configure real-time clock
  syncRtc();        // Sync RTC with GPS
  configureLora();  // Configure RFM95W
  //configureSd();    // Configure microSD
  //createLogFile();  // Create log file

  printLine(80);
  DEBUG_PRINTLN("RFM95W reliable datagram server");
  printDateTime();
  DEBUG_PRINTLN("Listening for messages...");

  printLine(80);
  blinkLed(10, 50);
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
      sprintf(tempBuffer, "Request from: 0x%02X\nSize: %d RSSI: %d SNR: %d",
              from, len, driver.lastRssi(), driver.lastSNR() );
      DEBUG_PRINTLN(tempBuffer);

      // Write incoming message buffer to union/structure
      for (int i = 0; i < len; ++i) {
        message.bytes[i] = buf[i];
      }

      // Print union/structure payload contents
      printUnion();
      printCsv();
      //printUnionHex();

      // Send reply back to the originator client
      uint8_t data[] = "OK";
      if (!manager.sendtoWait(data, sizeof(data), from)) {
        DEBUG_PRINTLN("sendtoWait failed");
      }
      blinkLed(2, 25);
    }
  }
}
