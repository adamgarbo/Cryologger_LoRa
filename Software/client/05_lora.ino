// ---------------------------------------
// RFM95W initialization defaults:
// ---------------------------------------
// Frequency:         434.0 MHz
// Power:             13 dBm
// Bandwidth:         125 kHz
// Coding Rate:       4/5
// Spreading Factor:  7 (128 chips/symbol)
// CRC:               ON
// ---------------------------------------

// Enable LoRa
void enableLora() {
  digitalWrite(PIN_SD_CS, HIGH); // Disable microSD SPI CS pin
  delay(1);
  digitalWrite(PIN_RF95_CS, LOW); // Enable LoRa SPI CS pin
  delay(1);
}

// Disable LoRa
void disableLora() {
  driver.sleep(); // Enable RFM95W sleep mode
  digitalWrite(PIN_RF95_CS, HIGH);  // Disable LoRa SPI CS pin
}

// Configure RFM95W radio
void configureLora() {

  // Initialize RFM95W
  if (!manager.init()) {
    DEBUG_PRINTLN("Warning: RFM95W initialization failed!");
  }

  // Set frequency
  if (!driver.setFrequency(RF95_FREQ)) {
    DEBUG_PRINTLN("Warning: setFrequency failed!");
  }

  // Set the transmitter power (2 to 20 dBm)
  driver.setTxPower(20, false);

  // Set the radio spreading factor (6 - 12)
  driver.setSpreadingFactor(RF95_SF);

  // Set the radio signal bandwidth (125.0, 250.0 or 500.0 MHz)
  driver.setSignalBandwidth(RF95_BW);

  // Set the coding rate to 4/5, 4/6, 4/7 or 4/8 (5, 6, 7 or 8)
  driver.setCodingRate4(RF95_CR);

  /// Turn on Cyclic Redundancy Check (CRC)
  driver.setPayloadCRC(RF95_CRC);

  // Wait until Channel Activity Detection shows no activity before transmitting
  driver.setCADTimeout(10000);

}

// Send LoRa data
void sendData() {

  DEBUG_PRINTLN("Sending message to server...");

  transmitCounter++;
  message.transmitCounter = transmitCounter; // Increment transmission counter

  // Send message to server
  if (manager.sendtoWait(message.bytes, sizeof(message), SERVER_ADDRESS)) {

    // Print contents of union
    printUnion();
    printUnionHex();

    // Wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;

    if (manager.recvfromAckTimeout(buf, &len, 2000, &from)) {

      char serverBuffer[100];
      sprintf(serverBuffer, "Reply from: 0x%02X Size: %d Payload: %s RSSI: %d SNR: %d\n",
              from, len, buf, driver.lastRssi(), driver.lastSNR());
      DEBUG_PRINT(serverBuffer);

      /*
        DEBUG_PRINT("Raw data: ");
        for (int i = 0; i < len; ++i) {
        DEBUG_PRINT_HEX(buf[i]);
        DEBUG_PRINT(" ");
        }
        DEBUG_PRINTLN();
      */

      message.rssi = driver.lastRssi();
      message.snr = driver.lastSNR();

      // Blink LED
      blinkLed(LED_GREEN, 2, 100);
    }
    else {
      DEBUG_PRINTLN("Warning: No reply! Is the server running?");
    }
  }
  else {
    DEBUG_PRINTLN("Warning: sendtoWait failed!");
  }

}
