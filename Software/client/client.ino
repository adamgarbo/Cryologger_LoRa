/*
  Cryologger: RFM95W Reliable Datagram Client

  Date: December 11, 2020

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
#define DEBUG false

#if DEBUG
#define DEBUG_PRINT(x)        Serial.print(x)
#define DEBUG_PRINTLN(x)      Serial.println(x)
#define DEBUG_PRINT_HEX(x)    Serial.print(x, HEX)
#define DEBUG_PRINTLN_HEX(x)  Serial.println(x, HEX)
#define DEBUG_PRINT_DEC(x)    Serial.println(x, DEC)
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

#define RFM95W_FREQ     915.0
#define PIN_RFM95W_CS   A2
#define PIN_RFM95W_INT  9

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
RH_RF95 driver(PIN_RFM95W_CS, PIN_RFM95W_INT);

// Class to manage message delivery and receipt using driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

// ----------------------------------------------------------------------------
// Global variables
// ----------------------------------------------------------------------------
volatile bool alarmFlag       = false;  // Alarm interrupt service routine flag
bool          ledState        = LOW;    // LED toggle flag
byte          alarmSeconds    = 0;     // Rolling alarm seconds
byte          alarmMinutes    = 1;      // Rolling alarm minutes
byte          alarmHours      = 0;      // Rolling alarm hours
unsigned long previousMillis  = 0;      // Global millis() timer

char          fileName[30]    = "";
char          outputData[100];          // Recording to SD in 512-byte chunks
char          tempData[50];             // Temporary SD data buffer

uint8_t data[] = "Hello World!";

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

// ----------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------
void setup() {

  // Pin assignments
  pinMode(PIN_VBAT, INPUT);
  pinMode(PIN_MICROSD_EN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_MICROSD_EN, LOW);
  digitalWrite(PIN_MICROSD_CS, HIGH);
  digitalWrite(PIN_RFM95W_CS, HIGH);

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
}

// ----------------------------------------------------------------------------
// Loop
// ----------------------------------------------------------------------------
void loop() {

  // Check if alarm flag was set
  if (alarmFlag) {
    alarmFlag = false;  // Clear alarm flag
    setRtcAlarm();      // Set RTC alarm

    DEBUG_PRINT("Alarm trigger: ");
    printDateTime();    // Print RTC date and time

    digitalWrite(LED_BUILTIN, HIGH);

    // Perform measurements
    readRtc();
    readBattery();

    // Transmit data
    sendData();

    // Log data
    configureSd();    // Re-initialize microSD
    logData();        // Write data to log file

    digitalWrite(LED_BUILTIN, LOW);

    disableLora();
    disableSd();
  }

  // Blink LED
  blinkLed(1, 25);

  // Enter deep sleep
  LowPower.deepSleep();
  //delay(1000);
}
