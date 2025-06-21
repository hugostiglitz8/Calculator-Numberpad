// NumberpadLogic.ino
#include "Keypad.h"  // for currentMode, etc.

void initNumberpad() {
  // TODO: initialize BLE HID when ready
}

void handleNumberpadMode(const char* key) {
  // Temporary stub: just print to serial until you wire up BLE
  Serial.print("NP key: ");
  Serial.println(key);
}