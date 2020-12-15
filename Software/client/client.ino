/*
  Cryologger: RFM95W Reliable Datagram Client

  Date: December 13, 2020

  Description:
  - Example sketch showing how to create a simple addressed, reliable messaging
  client with the RHReliableDatagram class, using the RH_RF95 driver to control
  a RF95 radio.
  - Designed to work with RF95 reliable datagram server
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
#define DEBUG_PRINTLN_BIN(x)  Serial.println(x, BIN)
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
#define PIN_RF95_CS       A2
#define PIN_VBAT          A5
#define PIN_FLASH_CS      8
#define PIN_RF95_INT      9
#define PIN_SD_CS    10
#define PIN_SD_EN    11
#define PIN_MOSI          19
#define PIN_SCK           20
#define PIN_MISO          21

// ----------------------------------------------------------------------------
// Object instantiations
// ----------------------------------------------------------------------------

RTCZero rtc;
SdFat   sd;    // File system object
SdFile  file;  // Log file

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
byte          alarmSeconds    = 0;      // Rolling alarm seconds
byte          alarmMinutes    = 1;      // Rolling alarm minutes
byte          alarmHours      = 0;      // Rolling alarm hours
unsigned long previousMillis  = 0;      // Global millis() timer
unsigned int  transmitCounter  = 0;      // LoRa transmission counter

char          fileName[30]    = "";
char          outputData[50];           // Recording to SD in 512-byte chunks

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];


// Union/structure to store and send data byte-by-byte via LoRa
typedef union {
  struct {
    uint32_t  unixtime;         // UNIX Epoch time      (4 bytes)
    float     voltage;          // Battery voltage (V)  (4 bytes)
    uint16_t  transmitCounter;  // Message counter      (2 bytes)
  } __attribute__((packed));
  uint8_t bytes[10]; // Size of structure (10 bytes)
} LoraPacket;

LoraPacket message;

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------
void setup() {

  // Pin assignments
  pinMode(PIN_VBAT, INPUT);
  pinMode(PIN_SD_EN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_SD_EN, LOW);
  digitalWrite(PIN_SD_CS, HIGH);


  digitalWrite(PIN_RF95_CS, HIGH);

  analogReadResolution(12); // Set analog resolution to 12-bits

  Serial.begin(115200);
  //while (!Serial) ; // Wait for serial port to be available
  delay(4000);

  DEBUG_PRINTLN("RFM95W Reliable Datagram Client");
  printDateTime();

  Wire.begin(); // Initialize I2C
  SPI.begin();  // Initialize SPI

  configureRtc();   // Configure real-time clock
  configureLora();  // Configure RFM95W
  configureSd();    // Configure microSD
  createLogFile();  // Create log file

  blinkLed(10, 50);
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
  blinkLed(1, 25);

  goToSleep();
  //delay(1000);
}
