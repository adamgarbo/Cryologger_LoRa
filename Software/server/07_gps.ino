// Configure GPS
void configureGps() {

  // Enable power to GPS
  digitalWrite(PIN_GPS_EN, LOW);

  // Start serial communications with GPS
  GpsSerial.begin(9600);
  blinkLed(2, 500);

  // Set NMEA port update rate to 1 Hz
  GpsSerial.println("$PMTK220,1000*1F");

  // Set NMEA sentence output frequencies to GGA and RMC
  GpsSerial.println("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");

  // Enable antenna updates
  //GpsSerial.println("$PGCMD,33,1*6C");

  // Disable antenna updates
  GpsSerial.println("$PGCMD,33,0*6D");

  // Disable power to GPS
  //digitalWrite(PIN_GPS_EN, HIGH);
}

// Read GPS
void readGps() {

  // Start loop timer
  unsigned long loopStartTime = millis();

  bool fixFound = false;
  bool charsSeen = false;
  byte fixCounter = 0;

  DEBUG_PRINTLN("Beginning to listen for GPS traffic...");

  // Look for GPS signal for up to 2 minutes
  while (!fixFound && millis() - loopStartTime < 2UL * 60UL * 1000UL) {
    if (GpsSerial.available()) {
      charsSeen = true;
      char c = GpsSerial.read();

      DEBUG_WRITE(c); // Echo NMEA sentences to serial

      if (gps.encode(c)) {
        // Check if GPS data is valid and up-to-date
        if ((gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) &&
            (gps.location.isUpdated() && gps.date.isUpdated() && gps.time.isUpdated())) {

          // Increment fix counter
          fixCounter++;

          char gpsDatetime[25];
          snprintf(gpsDatetime, sizeof(gpsDatetime), "%04u-%02d-%02d %02d:%02d:%02d,",
                   gps.date.year(), gps.date.month(), gps.date.day(),
                   gps.time.hour(), gps.time.minute(), gps.time.second());
          DEBUG_PRINT(gpsDatetime);
          DEBUG_PRINT(gps.location.lat()); DEBUG_PRINT(",");
          DEBUG_PRINT(gps.location.lng()); DEBUG_PRINT(",");
          DEBUG_PRINT(gps.satellites.value()); DEBUG_PRINT(",");
          DEBUG_PRINTLN(gps.hdop.hdop());

          // Check if minimum number of GPS fixes have been collected
          if (fixCounter >= 5) {
            fixFound = true;

            // Sync RTC with GPS time
            rtc.setTime(gps.time.hour(), gps.time.minute(), gps.time.second());
            rtc.setDate(gps.date.day(), gps.date.month(), gps.date.year() - 2000);

            //rtcSyncFlag = true;
            DEBUG_PRINT("RTC set: "); printDateTime();

            message.latitude = gps.location.lat();
            message.longitude = gps.location.lng();
          }
        }
      }
    }

    if ((millis() - loopStartTime) > 5000 && gps.charsProcessed() < 10) {
      DEBUG_PRINTLN("Warning: No GPS data received! Check wiring.");
      break;
    }
  }
  DEBUG_PRINTLN(charsSeen ? fixFound ?
                ("A GPS fix was found!") :
                ("No GPS fix was found.") :
                ("Wiring error: No GPS data seen."));

  // Disable power to GPS
  //digitalWrite(PIN_GPS_EN, HIGH);

  // Stop loop timer
  unsigned long loopEndTime = millis() - loopStartTime;
  DEBUG_PRINT("readGps() function execution: ");
  DEBUG_PRINT(loopEndTime); DEBUG_PRINTLN(" ms");

}

// Read GPS
void syncRtc() {

  // Start loop timer
  unsigned long loopStartTime = millis();

  bool fixFound = false;
  bool charsSeen = false;
  byte fixCounter = 0;

  DEBUG_PRINTLN("Attempting to sync RTC with GPS...");

  // Look for GPS signal for up to 5 minutes
  while (!fixFound && millis() - loopStartTime < 5UL * 60UL * 1000UL) {
    if (GpsSerial.available()) {
      charsSeen = true;
      char c = GpsSerial.read();

      DEBUG_WRITE(c); // Echo NMEA sentences to serial

      if (gps.encode(c)) {
        // Check if GPS data is valid and up-to-date
        if ((gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) &&
            (gps.location.isUpdated() && gps.date.isUpdated() && gps.time.isUpdated())) {

          // Increment fix counter
          fixCounter++;

          char gpsDatetime[25];
          snprintf(gpsDatetime, sizeof(gpsDatetime), "%04u-%02d-%02d %02d:%02d:%02d,",
                   gps.date.year(), gps.date.month(), gps.date.day(),
                   gps.time.hour(), gps.time.minute(), gps.time.second());
          DEBUG_PRINT(gpsDatetime);
          DEBUG_PRINT(gps.location.lat()); DEBUG_PRINT(",");
          DEBUG_PRINT(gps.location.lng()); DEBUG_PRINT(",");
          DEBUG_PRINT(gps.satellites.value()); DEBUG_PRINT(",");
          DEBUG_PRINTLN(gps.hdop.hdop());

          // Check if minimum number of GPS fixes have been collected
          if (fixCounter >= 5) {
            fixFound = true;

            // Sync RTC with GPS time
            rtc.setTime(gps.time.hour(), gps.time.minute(), gps.time.second());
            rtc.setDate(gps.date.day(), gps.date.month(), gps.date.year() - 2000);

            //rtcSyncFlag = true;
            DEBUG_PRINT("Success: RTC synced! "); printDateTime();
          }
        }
      }
    }

    if ((millis() - loopStartTime) > 5000 && gps.charsProcessed() < 10) {
      DEBUG_PRINTLN("Warning: No GPS data received! Check wiring.");
      break;
    }
  }
  DEBUG_PRINTLN(charsSeen ? fixFound ?
                ("A GPS fix was found!") :
                ("No GPS fix was found.") :
                ("Wiring error: No GPS data seen."));

  // Disable power to GPS
  //digitalWrite(PIN_GPS_EN, HIGH);

  // Stop loop timer
  unsigned long loopEndTime = millis() - loopStartTime;
  DEBUG_PRINT("readGps() function execution: ");
  DEBUG_PRINT(loopEndTime); DEBUG_PRINTLN(" ms");
}