#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <bluefruit.h>
#include "Fraction.h"
#include "Keypad.h"
#include "Display.h"

// Mode switch pin
const int MODE_SWITCH_PIN = 30;

// these live in Display.ino:
extern const int displayX, displayY, displayW, displayH;
extern String    lastDisplayCalc, lastDisplayHistory, lastDisplayAlternate;
extern bool      displayNeedsUpdate;
extern Adafruit_ILI9341 tft;

// Mode switching variables
Mode lastMode = MODE_CALCULATOR;
bool lastSwitchState = HIGH;
unsigned long lastSwitchTime = 0;
const unsigned long DEBOUNCE_DELAY = 50; // ms

void setup() {
  Serial.begin(115200);
  
  // Initialize mode switch pin
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  
  // Initialize display
  tft.begin();
  tft.setRotation(3);
  
  // Initialize keypad
  initKeypad();
  
  // Initialize calculator display
  drawInitialDisplay();
  
  // Initialize numberpad (BLE)
  initNumberpad();
  
  // Check initial mode
  checkModeSwitch();
  
  Serial.println("Ready - Calculator/Numberpad");
  Serial.print("Initial mode: ");
  Serial.println(currentMode == MODE_CALCULATOR ? "Calculator" : "Numberpad");
}

void loop() {
  keyScan();
  detectZeroHold();
  checkModeSwitch();
  processKeyBuffer();
  handleDisplay();
  handleModeDisplay();
}

void checkModeSwitch() {
  bool currentSwitchState = digitalRead(MODE_SWITCH_PIN);
  
  // Debounce the switch
  if (currentSwitchState != lastSwitchState) {
    lastSwitchTime = millis();
  }
  
  if ((millis() - lastSwitchTime) > DEBOUNCE_DELAY) {
    if (currentSwitchState != lastSwitchState) {
      // Switch state has changed and is stable
      if (currentSwitchState == LOW) {
        // Switch is pressed (pulled to ground)
        currentMode = MODE_NUMBERPAD;
      } else {
        // Switch is released (pulled up)
        currentMode = MODE_CALCULATOR;
      }
      
      // If mode changed, update display and BLE advertising
      if (currentMode != lastMode) {
        Serial.print("Mode changed to: ");
        Serial.println(currentMode == MODE_CALCULATOR ? "Calculator" : "Numberpad");
        
        if (currentMode == MODE_CALCULATOR) {
          drawInitialDisplay();
          stopNumberpadAdvertising(); // Stop advertising in calculator mode
        } else {
          startNumberpadAdvertising(); // Start advertising in numberpad mode
        }
        
        displayNeedsUpdate = true;
        lastMode = currentMode;
      }
    }
  }
  
  lastSwitchState = currentSwitchState;
}

