/*
  Cryologger: RFM95W Reliable Datagram Server

  Date: December 19, 2020

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
#define SERVER_ADDRESS 1
#define CLIENT_ADDRESS 2

#define RFM95W_FREQ   915.0
#define RFM95W_CS     A2
#define RFM95W_INT    9

// ----------------------------------------------------------------------------
// Object instantiations
// ----------------------------------------------------------------------------

// Singleton instance of the radio driver
RH_RF95 driver(RFM95W_CS, RFM95W_INT); // Moteino M0

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

// ----------------------------------------------------------------------------
// Global variables
// ----------------------------------------------------------------------------

uint8_t data[] = "Message received";

// Dont put this on the stack:
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

  // Initialize RFM95W
  if (!manager.init()) {
    DEBUG_PRINTLN("Warning: RFM95W initialization failed!");
  }

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz,
  // Cr = 4/5, Sf = 128chips/symbol, CRC on

  // Set frequency
  if (!driver.setFrequency(RFM95W_FREQ)) {
    DEBUG_PRINTLN("Warning: setFrequency failed");
  }

  // Set transmitter power (Range: 2 to 20 dBm)
  driver.setTxPower(20, false); // Default: 13 dBm

  // Wait until Channel Activity Detection shows no activity before transmitting
  //driver.setCADTimeout(10000);

  DEBUG_PRINTLN("RFM95W reliable datagram server");
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

      digitalWrite(LED_BUILTIN, HIGH);
      DEBUG_PRINT("Received request from: 0x");
      DEBUG_PRINTLN_HEX(from);
      DEBUG_PRINTLN();

      //RH_RF95::printBuffer("Raw data: ", buf, len);
      DEBUG_PRINT("Decoded buffer: ");
      Serial.println((char*)buf);
      DEBUG_PRINT("RSSI: ");
      DEBUG_PRINT_DEC(driver.lastRssi());

      DEBUG_PRINT("Raw data: ");
      for (int i = 0; i < len; ++i) {
        DEBUG_PRINT_HEX(buf[i]);
        DEBUG_PRINT(" ");
      }
      DEBUG_PRINTLN();

      // Send a reply back to the originator client
      if (!manager.sendtoWait(data, sizeof(data), from)) {
        DEBUG_PRINTLN("sendtoWait failed");
      }
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}
