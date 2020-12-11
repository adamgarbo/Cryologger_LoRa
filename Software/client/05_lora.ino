// Enable LoRa
void enableLora() {
  digitalWrite(PIN_MICROSD_EN, HIGH); // Disable microSD power
  digitalWrite(PIN_MICROSD_CS, HIGH); // Disable microSD SPI CS pin
  delay(1);
  digitalWrite(PIN_RFM95W_CS, LOW);   // Enable LoRa SPI CS pin
  delay(1);
}

void disableLora() {
  digitalWrite(PIN_RFM95W_CS, LOW);   // Enable LoRa SPI CS pin
  driver.sleep(); // Enable RFM95W sleep mode
}

// Initialize RFM95W
void configureLora() {

  enableLora(); // Enable LoRa and disable microSD

  if (!manager.init()) {
    DEBUG_PRINTLN("Warning: RFM95W initialization failed!");
  }

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5,
  // Sf = 128chips/symbol, CRC on

  // Set frequency
  if (!driver.setFrequency(RFM95W_FREQ)) {
    DEBUG_PRINTLN("Warning: setFrequency failed");
  }

  // Set transmitter power (Range: 2 to 20 dBm)
  driver.setTxPower(20, false); // Default: 13 dBm

  // Wait for Channel Activity Detection to show no activity before transmitting
  //driver.setCADTimeout(10000);
}

// Send LoRa data
void sendData() {

  DEBUG_PRINT("Sending message to server...");
  //DEBUG_PRINT("outputData size: "); DEBUG_PRINTLN(sizeof(outputData));

  // Send message to server
  if (manager.sendtoWait((uint8_t*)outputData, sizeof(outputData), SERVER_ADDRESS)) {

    // Wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
      DEBUG_PRINT(" reply received from: 0x");
      DEBUG_PRINTLN_HEX(from);
      DEBUG_PRINT("Server reply: ");
      DEBUG_PRINT((char*)buf);

      /*
        DEBUG_PRINT("Raw data: ");
        for (int i = 0; i < len; ++i) {
        DEBUG_PRINT_HEX(buf[i]);
        DEBUG_PRINT(" ");
        }
        DEBUG_PRINTLN();
      */
      DEBUG_PRINT(" RSSI: ");
      DEBUG_PRINTLN(driver.lastRssi());
    }
    else {
      DEBUG_PRINTLN("Warning: No reply! Is the server running?");
    }
  }
  else {
    DEBUG_PRINTLN("Warning: sendtoWait failed");
  }

  // Write data to SD buffer
  sprintf(tempData, "%d,\n", driver.lastRssi());
  strcat(outputData, tempData);

}
