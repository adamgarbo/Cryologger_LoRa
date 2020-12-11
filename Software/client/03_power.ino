// Read battery voltage
void readBattery() {

  // Start loop timer
  unsigned long loopStartTime = millis();

  float voltage = analogRead(A5) * 2.0 * (3.3 / 4096.0); // 0.003226 = 3.3 / 1023

  int vbat = voltage * 1000;
  // Write data to SD buffer
  sprintf(tempData, "%d,\n", vbat);
  strcat(outputData, tempData);

  //DEBUG_PRINT("voltage: "); DEBUG_PRINTLN(voltage);

  // Stop loop timer
  unsigned long loopEndTime = millis() - loopStartTime;
  //DEBUG_PRINT("readBattery() function execution: "); DEBUG_PRINT(loopEndTime); DEBUG_PRINTLN(" ms");
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
