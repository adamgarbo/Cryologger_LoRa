// Read battery voltage
void readBattery() {

  // Start loop timer
  unsigned long loopStartTime = millis();

  float voltage = analogRead(A5) * 2.0 * (3.3 / 4096.0);

  // Write data to union structure
  message.voltage = voltage;

  int vbat = voltage * 1000;

  // Write data to SD buffer
  char tempData[6];
  sprintf(tempData, "%d,", vbat);
  strcat(outputData, tempData);

  //DEBUG_PRINT("voltage: "); DEBUG_PRINTLN(voltage);

  // Stop loop timer
  unsigned long loopEndTime = millis() - loopStartTime;
  //DEBUG_PRINT("readBattery() function execution: ");
  //DEBUG_PRINT(loopEndTime);
  //DEBUG_PRINTLN(" ms");
}

// Blink LED (non-blocking)
void blinkLed(byte ledFlashes, unsigned int ledDelay) {

  pinMode(LED_BUILTIN, OUTPUT);
  byte i = 0;

  while (i < ledFlashes * 2) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > ledDelay) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
      i++;
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
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
  pinMode(PIN_MICROSD_CS, INPUT);
  digitalWrite(PIN_MOSI, LOW);
  digitalWrite(PIN_SCK, LOW);
  digitalWrite(PIN_MISO, LOW);
  digitalWrite(PIN_MICROSD_CS, LOW);
*/
  // Enter deep sleep and wait for WDT or RTC alarm interrupt
  LowPower.deepSleep();
}

void wakeUp() {
/*
  // Set SPI pins to low impedance state
  pinMode(PIN_MOSI, OUTPUT);
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_MISO, OUTPUT);
  pinMode(PIN_MICROSD_CS, OUTPUT);
  digitalWrite(PIN_MOSI, HIGH);
  digitalWrite(PIN_SCK, HIGH);
  digitalWrite(PIN_MISO, HIGH);
  digitalWrite(PIN_MICROSD_CS, HIGH);

  // Enable SPI
  SPI.begin();
*/
  // Disable LoRa
  enableLora();
}
