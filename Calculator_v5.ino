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
  debugSwitch(); 
  processKeyBuffer();
  handleDisplay();
  handleModeDisplay();
}

void debugSwitch() {
  static unsigned long lastDebug = 0;
  
  // Print switch state every 500ms
  if (millis() - lastDebug > 500) {
    bool switchState = digitalRead(MODE_SWITCH_PIN);
    Serial.print("Switch pin 30: ");
    Serial.print(switchState ? "HIGH" : "LOW");
    Serial.print(" | Current mode: ");
    Serial.println(currentMode == MODE_CALCULATOR ? "CALC" : "NUMPAD");
    lastDebug = millis();
  }
}

void checkModeSwitch() {
  bool currentSwitchState = digitalRead(MODE_SWITCH_PIN);
  
  // Debounce the switch
  if (currentSwitchState != lastSwitchState) {
    lastSwitchTime = millis();
  }
  
  if ((millis() - lastSwitchTime) > DEBOUNCE_DELAY) {
    // The switch state has been stable for the debounce period
    // Now check if the stable state is different from current mode
    
    Mode newMode;
    if (currentSwitchState == LOW) {
      // Switch is pressed (pulled to ground)
      newMode = MODE_NUMBERPAD;
    } else {
      // Switch is released (pulled up)
      newMode = MODE_CALCULATOR;
    }
    
    // Only update if mode actually changed
    if (newMode != currentMode) {
      currentMode = newMode;
      
      Serial.print("Mode changed to: ");
      Serial.println(currentMode == MODE_CALCULATOR ? "Calculator" : "Numberpad");
      
      if (currentMode == MODE_CALCULATOR) {
        drawInitialDisplay();
        stopNumberpadAdvertising(); // Stop advertising in calculator mode
      } else {
        startNumberpadAdvertising(); // Start advertising in numberpad mode
      }
      
      displayNeedsUpdate = true;
    }
  }
  
  lastSwitchState = currentSwitchState;
}

