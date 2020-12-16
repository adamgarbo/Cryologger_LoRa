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
  printLine(80);
  DEBUG_PRINTLN("Union/structure");
  printLine(80);
  DEBUG_PRINT("unixtime:"); printTab(1); DEBUG_PRINTLN(message.unixtime);
  DEBUG_PRINT("latitude:"); printTab(1); DEBUG_PRINTLN(message.latitude);
  DEBUG_PRINT("longitude:"); printTab(1); DEBUG_PRINTLN(message.longitude);
  DEBUG_PRINT("voltage:"); printTab(1); DEBUG_PRINTLN(message.voltage);
  DEBUG_PRINT("counter:"); printTab(1); DEBUG_PRINTLN(message.transmitCounter);
  printLine(80);
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
