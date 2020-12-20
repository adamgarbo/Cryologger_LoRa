// Enable microSD power and SPI CS
void enableSd() {

  digitalWrite(PIN_RF95_CS, HIGH);    // Disable LoRa SPI CS pin
  delay(1);
  digitalWrite(PIN_SD_CS, LOW);  // Enable microSD SPI CS pin
  delay(250);
}

// Disable power to microSD
void disableSd() {
  digitalWrite(PIN_RF95_CS, HIGH);    // Disable LoRa SPI CS pin
}

// Configure microSD
void configureSd() {

  enableSd(); // Enable microSD and disable LoRa

  // Initialze microSD
  if (!sd.begin(PIN_SD_CS, SD_SCK_MHZ(4))) {
    DEBUG_PRINTLN("Warning: microSD not detected! Please check wiring.");
  }
}

// Create log file
void createLogFile() {

  if (file.isOpen()) {
    file.close();
  }

  // Create log file name
  sprintf(fileName, "20%02d%02d%02d_%02d%02d%02d.csv",
          rtc.getYear(), rtc.getMonth(), rtc.getDay(),
          rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());

  // O_CREAT - Create the file if it does not exist
  // O_APPEND - Seek to the end of the file prior to each write
  // O_WRITE - Open the file for writing
  if (!file.open(fileName, O_CREAT | O_APPEND | O_WRITE)) {
    DEBUG_PRINTLN("Failed to create log file");
    return;
  }

  if (!file.isOpen()) {
    DEBUG_PRINTLN("Warning: Unable to open file");
  }

  // Update file create timestamp
  updateFileCreate();

  // Write header to file
  file.println("unixtime,latitude,longitude,satellites,hdop,voltage,rssi,snr,counter");

  // Sync the log file
  file.sync();

  // Close log file
  file.close();

  DEBUG_PRINT("Logging to file: "); DEBUG_PRINTLN(fileName);
}

// Log data to microSD
void logData() {

  // Open log file and append data
  if (file.open(fileName, O_APPEND | O_WRITE)) {
    file.print(message.unixtime); file.print(",");
    file.print(message.latitude, 6); file.print(",");
    file.print(message.longitude, 6); file.print(",");
    file.print(message.satellites); file.print(",");
    file.print(message.hdop); file.print(",");
    file.print(message.voltage, 2); file.print(",");
    file.print(message.rssi); file.print(",");
    file.print(message.snr); file.print(",");
    file.println(message.transmitCounter);
    updateFileAccess(); // Update file access and write timestamps
  }
  else {
    DEBUG_PRINTLN("Warning: Unable to open file!");
  }

  // Sync log file
  if (!file.sync()) {
    DEBUG_PRINTLN(F("Warning: File sync error!"));
  }

  // Check for write error
  if (file.getWriteError()) {
    DEBUG_PRINTLN(F("Warning: File write error!"));
  }

  // Close log file
  if (!file.close()) {
    DEBUG_PRINTLN(F("Warning: File close error!"));
  }

  // Blink LED
  blinkLed(LED_RED, 2, 100);

}

// Update the file create timestamp
void updateFileCreate() {

  if (!file.timestamp(T_CREATE, rtc.getYear() + 2000, rtc.getMonth(),
                      rtc.getDay(), rtc.getHours(), rtc.getMinutes(),
                      rtc.getSeconds())) {
    DEBUG_PRINTLN("Warning: Unable to write file create timestamp");
  }
}

// Update the file access and write timestamps
void updateFileAccess() {

  if (!file.timestamp(T_ACCESS, rtc.getYear() + 2000, rtc.getMonth(),
                      rtc.getDay(), rtc.getHours(), rtc.getMinutes(),
                      rtc.getSeconds()))
  {
    DEBUG_PRINTLN("Warning: Unable to write file access timestamp");
  }
  if (!file.timestamp(T_WRITE, rtc.getYear() + 2000, rtc.getMonth(),
                      rtc.getDay(), rtc.getHours(), rtc.getMinutes(),
                      rtc.getSeconds())) {
    DEBUG_PRINTLN("Warning: Unable to write file write timestamp");
  }
}
