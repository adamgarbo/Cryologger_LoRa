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
  digitalWrite(PIN_FLASH_CS, HIGH); // Disable Flash SPI CS pin
  delay(1);
  digitalWrite(PIN_RF95_CS, LOW);   // Enable LoRa SPI CS pin
  delay(1);
}

// Disable LoRa
void disableLora() {
  driver.sleep();                   // Enable RFM95W sleep mode
  digitalWrite(PIN_RF95_CS, HIGH);  // Disable LoRa SPI CS pin
}

// Configure RFM95W radio
void configureLora() {

  // Enable LoRa
  enableLora(); 

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

  // Check Channel Activity Detection before transmitting
  driver.setCADTimeout(10000);

}
