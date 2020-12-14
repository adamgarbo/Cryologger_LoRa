// Enable LoRa
void enableLora() {
  digitalWrite(PIN_MICROSD_EN, HIGH); // Disable microSD power
  digitalWrite(PIN_MICROSD_CS, HIGH); // Disable microSD SPI CS pin
  delay(1);
  digitalWrite(PIN_RF95_CS, LOW);   // Enable LoRa SPI CS pin
  delay(1);
}

// Disable LoRa
void disableLora() {
  driver.sleep();                   // Enable RFM95W sleep mode
  digitalWrite(PIN_RF95_CS, HIGH);  // Disable LoRa SPI CS pin
}

// Initialize RFM95W
void configureLora() {

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

  // Set the coding rate (4/5, 4/6, 4/7 or 4/8)
  driver.setCodingRate4(RF95_CR);

  /// Turn on Cyclic Redundancy Check (CRC)
  driver.setPayloadCRC(RF95_CRC);

  // Wait until Channel Activity Detection shows no activity before transmitting
  driver.setCADTimeout(10000);

}

          
void printBuffer(uint8_t* message) {

  // Print payload in hexadecimal
  DEBUG_PRINT(" Raw payload: ");
  for (int i = 0; i < sizeof(message); ++i) {
    DEBUG_PRINT_HEX(message[i]);
    DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
}
