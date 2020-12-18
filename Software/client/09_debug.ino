void printLine(byte dashes) {
  for (byte i = 0; i < dashes; i++) {
    DEBUG_PRINT("-");
  }
  DEBUG_PRINTLN();
}

void printTab(byte tabs) {
  for (byte i = 0; i < tabs; i++) {
    DEBUG_PRINT("\t");
  }
}

// Print contents of union/structure
void printUnion() {
  printLine(30);
  DEBUG_PRINTLN("Payload:");
  printLine(30);
  DEBUG_PRINT("unixtime:");   printTab(1);  DEBUG_PRINTLN(message.unixtime);
  DEBUG_PRINT("latitude:");   printTab(1);  DEBUG_PRINTLN_DEC(message.latitude, 8);
  DEBUG_PRINT("longitude:");  printTab(1);  DEBUG_PRINTLN_DEC(message.longitude, 8);
  DEBUG_PRINT("satellites:"); printTab(1);  DEBUG_PRINTLN(message.satellites);
  DEBUG_PRINT("hdop:");       printTab(2);  DEBUG_PRINTLN(message.hdop);
  DEBUG_PRINT("voltage:");    printTab(1);  DEBUG_PRINTLN_DEC(message.voltage, 2);
  DEBUG_PRINT("counter:");    printTab(1);  DEBUG_PRINTLN(message.transmitCounter);
  printLine(30);
}

// Print contents of union/structure in hexadecimal
void printUnionHex() {
  DEBUG_PRINTLN("Hex");
  printLine(80);
  char tempData[20];
  DEBUG_PRINTLN("Byte\tHex");
  for (int i = 0; i < sizeof(message); ++i) {
    sprintf(tempData, "%d\t0x%02X", i, message.bytes[i]);
    DEBUG_PRINTLN(tempData);
  }
  printLine(80);
}
