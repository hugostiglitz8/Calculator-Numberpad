// NumberpadLogic.ino
#include "Keypad.h"
#include <bluefruit.h>

BLEDis bledis;
BLEHidAdafruit blehid;
bool bleInitialized = false;

// Keycode mapping for HID (standard USB HID keycodes)
const uint8_t keyHIDMap[5][5] = {
  { HID_KEY_ESCAPE, HID_KEY_ARROW_LEFT, HID_KEY_ARROW_RIGHT, HID_KEY_KEYPAD_DIVIDE, HID_KEY_BACKSPACE },      // AC=ESC, >=LEFT, x/y=RIGHT, /, Del=BACKSPACE
  { HID_KEY_KEYPAD_7, HID_KEY_KEYPAD_8, HID_KEY_KEYPAD_9, HID_KEY_KEYPAD_MULTIPLY, HID_KEY_V }, // 7,8,9,x,MM=CMD+V
  { HID_KEY_KEYPAD_4, HID_KEY_KEYPAD_5, HID_KEY_KEYPAD_6, HID_KEY_KEYPAD_SUBTRACT, HID_KEY_C }, // 4,5,6,-,round=CMD+C
  { HID_KEY_KEYPAD_1, HID_KEY_KEYPAD_2, HID_KEY_KEYPAD_3, HID_KEY_KEYPAD_ADD, HID_KEY_ENTER }, // 1,2,3,+,return=ENTER
  { HID_KEY_KEYPAD_0, HID_KEY_KEYPAD_0, HID_KEY_KEYPAD_DECIMAL, HID_KEY_KEYPAD_ADD, HID_KEY_ENTER } // 0,0,.,+,return=ENTER
};

void initNumberpad() {
  if (!bleInitialized) {
    Serial.println("Initializing BLE HID Keyboard");
    
    // Initialize Bluefruit
    Bluefruit.begin();
    Bluefruit.setTxPower(4);
    Bluefruit.setName("Calculator Numberpad");
    
    // Configure and Start Device Information Service
    bledis.setManufacturer("Your Company");
    bledis.setModel("Calculator Numberpad");
    bledis.begin();

    // Start BLE HID
    blehid.begin();
    
    // Set up connection callbacks
    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
    
    bleInitialized = true;
  }
}

void startNumberpadAdvertising() {
  if (!bleInitialized) return;
  
  Serial.println("Starting BLE advertising");
  
  // Clear advertising data first
  Bluefruit.Advertising.clearData();
  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();
  
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}

void stopNumberpadAdvertising() {
  if (!bleInitialized) return;
  
  Serial.println("Stopping BLE advertising");
  Bluefruit.Advertising.stop();
}

void connect_callback(uint16_t conn_handle) {
  Serial.println("Connected to device");
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  Serial.println("Disconnected from device");
  
  // Only restart advertising if we're still in numberpad mode
  if (currentMode == MODE_NUMBERPAD) {
    startNumberpadAdvertising();
  }
}

void handleNumberpadMode(const char* key) {
  // Only send keys if we're connected
  if (!Bluefruit.connected()) {
    Serial.println("Not connected to BLE device");
    return;
  }

  // Find the key in our keymap
  uint8_t hidCode = 0;
  uint8_t modifier = 0;
  
  for (int r = 0; r < 5; r++) {
    for (int c = 0; c < 5; c++) {
      if (strcmp(keymap[r][c], key) == 0) {
        hidCode = keyHIDMap[r][c];
        
        // Special handling for CMD+C and CMD+V
        if ((r == 1 && c == 4) || (r == 2 && c == 4)) { // MM or round keys
          modifier = KEYBOARD_MODIFIER_LEFTGUI; // CMD key on Mac / Windows key on PC
        }
        // Special handling for arrow keys when slash is held (shift mode)
        else if (slashHoldActive && (strcmp(key, ">") == 0 || strcmp(key, "x/y") == 0)) {
          modifier = KEYBOARD_MODIFIER_LEFTSHIFT; // Add shift to arrow keys
          Serial.print("Shift + ");
        }
        break;
      }
    }
    if (hidCode != 0) break;
  }

  // Send the key if we found a valid mapping
  if (hidCode != 0) {
    Serial.print("Sending HID key: ");
    Serial.print(key);
    if (modifier == KEYBOARD_MODIFIER_LEFTGUI) Serial.print(" (with CMD)");
    if (modifier == KEYBOARD_MODIFIER_LEFTSHIFT) Serial.print(" (with SHIFT)");
    Serial.println();
    
    // Create keycode array (up to 6 keys can be pressed simultaneously)
    uint8_t keycodes[6] = { hidCode, 0, 0, 0, 0, 0 };
    
    // Send key press (modifier, keycodes array)
    blehid.keyboardReport(modifier, keycodes);
    delay(10); // Small delay
    
    // Send key release (all zeros)
    blehid.keyRelease();
  } else {
    Serial.print("No HID mapping for key: ");
    Serial.println(key);
  }
}