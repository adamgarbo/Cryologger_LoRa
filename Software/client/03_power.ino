// Read battery voltage
void readBattery() {

  // Start loop timer
  unsigned long loopStartTime = millis();

  float voltage = analogRead(PIN_VBAT);
  voltage *= 2;    // Divided by 2, so multiply back
  voltage *= 3.3;  // Multiply by 3.3V, our reference voltage
  voltage /= 4096.0; // convert to voltage

  // Write data to union structure
  message.voltage = voltage;

  DEBUG_PRINT("voltage: "); DEBUG_PRINTLN(voltage);

  // Stop loop timer
  unsigned long loopEndTime = millis() - loopStartTime;
  DEBUG_PRINT("readBattery() function execution: ");
  DEBUG_PRINT(loopEndTime);
  DEBUG_PRINTLN(" ms");
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

  // Disable microSD card
  disableSd();
  /*
    // Disable SPI
    SPI.end();

    // Set SPI pins to high impedance state
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_SCK, INPUT);
    pinMode(PIN_MISO, INPUT);
    pinMode(PIN_SD_CS, INPUT);
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_SCK, LOW);
    digitalWrite(PIN_MISO, LOW);
    digitalWrite(PIN_SD_CS, HIGH);
  */
  // Enter deep sleep and wait for WDT or RTC alarm interrupt
  LowPower.deepSleep();
  //delay(1000);
}

void wakeUp() {
  /*
    // Set SPI pins to low impedance state
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_SCK, OUTPUT);
    pinMode(PIN_MISO, OUTPUT);
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_MOSI, HIGH);
    digitalWrite(PIN_SCK, HIGH);
    digitalWrite(PIN_MISO, HIGH);
    digitalWrite(PIN_SD_CS, LOW);

    // Enable SPI
    SPI.begin();
  */
  // Disable LoRa
  enableLora();
}
