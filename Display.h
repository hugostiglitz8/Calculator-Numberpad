// Display.h


#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// (these are defined in Display.ino)
extern const int        displayX, displayY, displayW, displayH;
extern String           lastDisplayCalc, lastDisplayHistory, lastDisplayAlternate;
extern bool             displayNeedsUpdate;
extern Adafruit_ILI9341 tft;

void drawInitialDisplay();
void updateDisplay();
void handleDisplay();
void clearDisplayArea();

#endif // DISPLAY_H