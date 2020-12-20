/*
  Cryologger: RFM95W Reliable Datagram Client

  Date: December 13, 2020

  Description:
  - Example sketch showing how to create a simple addressed, reliable messaging
  client with the RHReliableDatagram class, using the RH_RF95 driver to control
  a RF95 radio.
  - Designed to work with RF95 reliable datagram server

  - Sleep current of 260 uA measured December 17, 2020
*/

// ----------------------------------------------------------------------------
// Libraries
// ----------------------------------------------------------------------------
#include <ArduinoLowPower.h>      // https://github.com/arduino-libraries/ArduinoLowPower
#include <RHReliableDatagram.h>   // https://airspayce.com/mikem/arduino/RadioHead/
#include <RH_RF95.h>              // https://airspayce.com/mikem/arduino/RadioHead/
#include <RTCZero.h>              // https://github.com/arduino-libraries/RTCZero
#include <SdFat.h>                // https://github.com/greiman/SdFat
#include <SPI.h>
#include <TinyGPS++.h>            // https://github.com/mikalhart/TinyGPSPlus
#include <Wire.h>

// ----------------------------------------------------------------------------
// Debugging macros
// ----------------------------------------------------------------------------
#define DEBUG   true
#define MICROSD true

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
// Port definitions
// ----------------------------------------------------------------------------
#define GpsSerial     Serial1

// ----------------------------------------------------------------------------
// Definitions
// ----------------------------------------------------------------------------
#define SERVER_ADDRESS  1
#define CLIENT_ADDRESS  2

// ----------------------------------------------------------------------------
// RFM95W radio definitions
// ----------------------------------------------------------------------------
#define RF95_FREQ     915.0   // Radio frequency (MHz)
#define RF95_PW       20      // Transmit power (dBm)
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
#define LED_GREEN     8
#define PIN_VBAT      9
#define LED_RED       13
#define PIN_MISO      22
#define PIN_MOSI      23
#define PIN_SCK       24

// ----------------------------------------------------------------------------
// Object instantiations
// ----------------------------------------------------------------------------
RTCZero     rtc;
SdFat       sd;
SdFile      file;
TinyGPSPlus gps;

// Singleton instance of the radio driver
RH_RF95 driver(PIN_RF95_CS, PIN_RF95_INT);

// Class to manage message delivery and receipt using driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

// ----------------------------------------------------------------------------
// Global variables
// ----------------------------------------------------------------------------
volatile bool alarmFlag       = false;  // RTC alarm ISR flag
volatile bool watchdogFlag    = false;  // Watchdog Timer ISR flag
volatile int  watchdogCounter = 0;      // Watchdog Timer interrupt counter
bool          ledState        = LOW;    // LED toggle flag
bool          rtcSyncFlag     = false;  // RTC synchronization flag
byte          alarmSeconds    = 10;      // Rolling alarm seconds
byte          alarmMinutes    = 0;      // Rolling alarm minutes
byte          alarmHours      = 0;      // Rolling alarm hours
unsigned long previousMillis  = 0;      // Global millis() timer
unsigned int  transmitCounter = 0;      // LoRa transmission counter
char          fileName[30]    = "";

// Dont put this on the stack:
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
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_GPS_EN, OUTPUT);
  //pinMode(PIN_RF95_RST, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_GPS_EN, LOW);
  digitalWrite(PIN_SD_CS, HIGH);
  digitalWrite(PIN_RF95_CS, HIGH);
  //digitalWrite(PIN_RF95_RST, HIGH);

  analogReadResolution(12); // Set analog resolution to 12-bits

  Serial.begin(115200);
  //while (!Serial) ; // Wait for serial port to be available
  delay(4000);

  printLine(80);
  DEBUG_PRINTLN("RFM95W Reliable Datagram Client");
  printLine(80);

  Wire.begin(); // Initialize I2C
  SPI.begin();  // Initialize SPI

  configureRtc();   // Configure real-time clock
  enableGps();      // Enable GPS
  syncRtc();        // Sync RTC with GPS
  configureLora();  // Configure RFM95W radio
  configureSd();    // Configure microSD
  createLogFile();  // Create log file

  setRtcAlarm1();   // Set initial alarm for minute rollover

  blinkLed(LED_GREEN, 5, 100);
  blinkLed(LED_RED, 5, 100);

}

// ----------------------------------------------------------------------------
// Loop
// ----------------------------------------------------------------------------
void loop() {

  // Check if alarm flag was set
  if (alarmFlag) {
    wakeUp();
    alarmFlag = false;  // Clear alarm flag
    setRtcAlarm();      // Set RTC alarm

    DEBUG_PRINT("Alarm trigger: ");
    printDateTime();    // Print RTC date and time

    // Perform measurements
    readRtc();
    readBattery();
    readGps();

    // Transmit data
    sendData();

    // Log data
    configureSd();    // Re-initialize microSD
    logData();        // Write data to log file
  }

  // Check if watchdog flag was set
  if (watchdogFlag) {
    petDog(); // Reset the Watchdog Timer
  }

  // Blink LED
  blinkLed(LED_RED, 1, 25);

  goToSleep();

}
