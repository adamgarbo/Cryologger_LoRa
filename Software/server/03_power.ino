// Read battery voltage
void readBattery() {

  // Start loop timer
  unsigned long loopStartTime = millis();

  // Measure voltage
  voltage = analogRead(A5) * 2.0 * (3.3 / 4096.0); // 0.003226 = 3.3 / 1023

  DEBUG_PRINT("voltage: "); DEBUG_PRINTLN(voltage);

  // Stop loop timer
  unsigned long loopEndTime = millis() - loopStartTime;
  //DEBUG_PRINT("readBattery() function execution: ");
  //DEBUG_PRINT(loopEndTime);
  //DEBUG_PRINTLN(" ms");
}

// Blink LED (non-blocking)
void blinkLed(byte ledPin, byte ledFlashes, unsigned int ledDelay) {

  pinMode(ledPin, OUTPUT);
  byte i = 0;

  while (i < ledFlashes * 2) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > ledDelay) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      i++;
    }
  }
  digitalWrite(ledPin, LOW);
}

void goToSleep() {

  // Disable LoRa
  disableLora();

  // Disable Flash
  //disableFlash();

  /*
    // Disable SPI
    SPI.end();

    // Set SPI pins to high impedance state
    pinMode(PIN_FLASH_CS, INPUT);
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_SCK, INPUT);
    pinMode(PIN_MISO, INPUT);
    pinMode(PIN_MICROSD_CS, INPUT);
    digitalWrite(PIN_FLASH_CS, LOW);
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_SCK, LOW);
    digitalWrite(PIN_MISO, LOW);
    digitalWrite(PIN_MICROSD_CS, LOW);

  */
  // Enter deep sleep and wait for WDT or RTC alarm interrupt
  LowPower.deepSleep();
}

void wakeUp() {

  enableLora();
  /*
    // Set SPI pins to low impedance state
    pinMode(PIN_FLASH_CS, OUTPUT);
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_SCK, OUTPUT);
    pinMode(PIN_MISO, OUTPUT);
    pinMode(PIN_MICROSD_CS, OUTPUT);
    digitalWrite(PIN_FLASH_CS, HIGH);
    digitalWrite(PIN_MOSI, HIGH);
    digitalWrite(PIN_SCK, HIGH);
    digitalWrite(PIN_MISO, HIGH);
    digitalWrite(PIN_MICROSD_CS, HIGH);

    // Enable SPI
    SPI.begin();
  */
}
