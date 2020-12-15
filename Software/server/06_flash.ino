// Enable LoRa
void enableFlash() {
  digitalWrite(PIN_RF95_CS, HIGH); // Disable LoRa SPI CS pin
  delay(1);
  digitalWrite(PIN_FLASH_CS, LOW); // Enable Flash SPI CS pin
  delay(1);
}

// Disable Flash
void disableFlash() {
  flash.sleep(); // Enable Flash sleep mode
  digitalWrite(PIN_FLASH_CS, HIGH); // Disable Flash SPI CS pin
}

// Configure flash
void configureFlash() {

  // Enable Flash
  enableFlash();

  // Initialize Flash
  if (!flash.initialize()) {
    Serial.println("Warning: Flash initialization failed!");
  }
  else {
    flash.sleep(); // Enter sleep mode
  }
}
