// Configure real-time clock
void configureRtc() {

  // Initialize RTC
  rtc.begin();

  // Manually set RTC time and date
  //rtc.setTime(12, 59, 55); // (hours, minutes, seconds)
  //rtc.setDate(10, 12, 20); // (day, month, year)

  // Set alarm to occur on next hour rollover
  rtc.setAlarmTime(0, 0, 0);

  // Enable alarm
  rtc.enableAlarm(rtc.MATCH_SS);

  // Attach alarm to interrupt service routine
  rtc.attachInterrupt(alarmIsr);
}


// Read RTC's date and time
void readRtc() {

  // Start loop timer
  unsigned long loopStartTime = millis();

  // Get UNIX Epoch time
  unsigned long unixtime = rtc.getEpoch();

  // Write data to SD buffer
  char tempData[30];
  sprintf(tempData, "20%02d-%02d-%02d %02d:%02d:%02d,%ld,",
          rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHours(),
          rtc.getMinutes(), rtc.getSeconds(), unixtime);
  strcat(outputData, tempData);

  // Stop loop timer
  unsigned long loopEndTime = millis() - loopStartTime;
  //DEBUG_PRINT("readRtc() function execution: "); 
  //DEBUG_PRINT(loopEndTime); 
  //DEBUG_PRINTLN(" ms");
}

// Set RTC rolling alarm
void setRtcAlarm() {

//(rtc.getSeconds() + alarmSeconds) % 60
  rtc.setAlarmTime((rtc.getHours() + alarmHours) % 24,
                   (rtc.getMinutes() + alarmMinutes) % 60,
                   0);
  rtc.setAlarmDate(rtc.getDay(), rtc.getMonth(), rtc.getYear());

  rtc.enableAlarm(rtc.MATCH_SS);

  // Attach alarm to interrupt service routine
  rtc.attachInterrupt(alarmIsr);
}

// RTC alarm interrupt service routine
void alarmIsr() {
  alarmFlag = true; // Set alarm flag
}

// Print the RTC's current date and time
void printDateTime() {
  char dateTimeBuffer[25];
  sprintf(dateTimeBuffer, "20%02d-%02d-%02d %02d:%02d:%02d",
          rtc.getYear(), rtc.getMonth(), rtc.getDay(),
          rtc.getHours(), rtc.getMinutes(), rtc.getSeconds());
  DEBUG_PRINTLN(dateTimeBuffer);
}

// Print the RTC's alarm
void printAlarm() {
  char alarmBuffer[25];
  sprintf(alarmBuffer, "20%02d-%02d-%02d %02d:%02d:%02d",
          rtc.getAlarmYear(), rtc.getAlarmMonth(), rtc.getAlarmDay(),
          rtc.getAlarmHours(), rtc.getAlarmMinutes(), rtc.getAlarmSeconds());
  DEBUG_PRINTLN(alarmBuffer);
}
