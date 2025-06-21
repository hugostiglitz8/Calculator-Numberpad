#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Fraction.h"
#include "Keypad.h"         // <— brings in everything keyScan needs
#include "Display.h"

// these live in Display.ino:
extern const int displayX, displayY, displayW, displayH;
extern String    lastDisplayCalc, lastDisplayHistory, lastDisplayAlternate;
extern bool      displayNeedsUpdate;
extern Adafruit_ILI9341 tft;

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(3);
  initKeypad();           // from Keypad.h / keyScan.ino
  drawInitialDisplay();   // from Display.ino
  Serial.println("Ready");
}

void loop() {
  keyScan();
  detectZeroHold();
  processKeyBuffer();
  handleDisplay();
}

