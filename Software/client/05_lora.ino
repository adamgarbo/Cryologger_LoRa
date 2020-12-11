// Enable LoRa
void enableLora() {
  digitalWrite(PIN_MICROSD_EN, HIGH);
  digitalWrite(PIN_MICROSD_CS, HIGH);
  delay(1);
  digitalWrite(RFM95W_CS, LOW);
  delay(1);
}

// Initialize RFM95W
void configureLora() {

  enableLora();

  if (!manager.init()) {
    DEBUG_PRINTLN("Warning: RFM95W initialization failed!");
  }
  else {
    DEBUG_PRINTLN("RFM95W initializated.");
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

  DEBUG_PRINTLN("Sending to RF95 reliable datagram server...");
  DEBUG_PRINT("outputData size: "); DEBUG_PRINTLN(sizeof(outputData));

  // Send message to server
  if (manager.sendtoWait((uint8_t*)outputData, sizeof(outputData), SERVER_ADDRESS)) {

    // Wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
      DEBUG_PRINT("Received reply from: 0x");
      DEBUG_PRINTLN_HEX(from);
      DEBUG_PRINT("Decoded message: ");
      DEBUG_PRINTLN((char*)buf);
      //RH_RF95::printBuffer("Raw data: ", buf, len);

      DEBUG_PRINT("Raw data: ");
      for (int i = 0; i < len; ++i) {
        DEBUG_PRINT_HEX(buf[i]);
        DEBUG_PRINT(" ");
      }
      DEBUG_PRINTLN();
    }
    else {
      DEBUG_PRINTLN("Warning: No reply! Is the server running?");
    }
  }
  else {
    DEBUG_PRINTLN("Warning: sendtoWait failed");
  }

  // Clear arrays
  memset(outputData, 0x00, sizeof(outputData));
  memset(tempData, 0x00, sizeof(tempData));
}
