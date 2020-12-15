void printLine() {
  for (byte i = 0; i < 79; i++) {
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
  printLine();
  DEBUG_PRINTLN("Payload:");
  printLine();
  DEBUG_PRINT("unixtime:"); printTab(1); DEBUG_PRINTLN(message.unixtime);
  DEBUG_PRINT("voltage:"); printTab(1); DEBUG_PRINTLN(message.voltage);
  DEBUG_PRINT("counter:"); printTab(1); DEBUG_PRINTLN(message.transmitCounter);
  printLine();
}

// Print contents of union/structure in hexadecimal
void printUnionHex() {
  DEBUG_PRINTLN("Payload (Hex):");
  printLine();
  char tempData[20];
  DEBUG_PRINTLN("Byte\tHex");
  for (int i = 0; i < sizeof(message); ++i) {
    sprintf(tempData, "%d\t0x%02X", i, message.bytes[i]);
    DEBUG_PRINTLN(tempData);
  }
  printLine();
}
