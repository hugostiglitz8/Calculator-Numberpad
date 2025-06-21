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
  detectSlashHold();  // Add slash hold detection
  checkModeSwitch();
  processKeyBuffer();
  handleDisplay();
  handleModeDisplay();
}

void checkModeSwitch() {
  bool currentSwitchState = digitalRead(MODE_SWITCH_PIN);
  
  // Simple debouncing: only check for changes after the debounce period
  if (currentSwitchState != lastSwitchState) {
    lastSwitchTime = millis(); // Reset the debounce timer
    lastSwitchState = currentSwitchState;
    return; // Exit early, wait for next loop iteration
  }
  
  // If we get here, the switch state hasn't changed since last time
  // Check if enough time has passed for debouncing
  if ((millis() - lastSwitchTime) < DEBOUNCE_DELAY) {
    return; // Still within debounce period, don't act yet
  }
  
  // Switch state is stable, determine desired mode
  Mode desiredMode = (currentSwitchState == LOW) ? MODE_NUMBERPAD : MODE_CALCULATOR;
  
  // Only update if mode needs to change
  if (desiredMode != currentMode) {
    currentMode = desiredMode;
    
    Serial.print("Mode switched to: ");
    Serial.println(currentMode == MODE_CALCULATOR ? "Calculator" : "Numberpad");
    
    if (currentMode == MODE_CALCULATOR) {
      drawInitialDisplay();
      stopNumberpadAdvertising();
    } else {
      startNumberpadAdvertising();
    }
    
    displayNeedsUpdate = true;
  }
}
