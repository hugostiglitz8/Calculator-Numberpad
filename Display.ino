//Display.ino


#include "Display.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <bluefruit.h>  // Add this line

// only include each font once
#include <Arimo_VariableFont_wght25pt7b.h>
#include <Arimo_VariableFont_wght20pt7b.h>
#include <Arimo_VariableFont_wght15pt7b.h>
#include <Arimo_VariableFont_wght12pt7b.h>
#include <Arimo_VariableFont_wght10pt7b.h>
#include <Arimo_VariableFont_wght7pt7b.h>
#include <Arimo_VariableFont_wght5pt7b.h>
#include <Arimo_VariableFont_wght3pt7b.h>

#define ILI9341_RUBY 0x8800 
//#define ILI9341_BACKGROUND 0x662C

const uint16_t ruby = ILI9341_RUBY;
const uint16_t backgroundColor = 0x4208;
//const uint16_t backgroundColor = 0x31A6;

const int   displayX             = 0;
const int   displayY             = 30;
const int   displayW             = 320;
const int   displayH             = 210;

String      lastDisplayCalc      = "";
String      lastDisplayHistory   = "";
String      lastDisplayAlternate = "";

bool        displayNeedsUpdate   = false;

extern Adafruit_ILI9341 tft;



#define TFT_CS   27
#define TFT_DC   25
#define TFT_RST  26
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);


void handleModeDisplay() {
  static unsigned long lastModeUpdate = 0;
  static bool modeDisplayVisible = false;
  static Mode lastDisplayedMode = MODE_CALCULATOR;
  
  // Show mode indicator for 2 seconds after mode change
  if (currentMode != lastDisplayedMode || (millis() - lastModeUpdate < 2000)) {
    if (!modeDisplayVisible || currentMode != lastDisplayedMode) {
      // Draw mode indicator
      tft.fillRect(220, 5, 95, 20, ILI9341_BLACK);
      tft.setFont();
      tft.setTextSize(1);
      tft.setTextColor(currentMode == MODE_CALCULATOR ? ILI9341_WHITE : ILI9341_CYAN);
      tft.setCursor(225, 10);
      tft.print(currentMode == MODE_CALCULATOR ? "CALC" : "NUMPAD");
      
      // Show connection status for numberpad mode
      if (currentMode == MODE_NUMBERPAD) {
        tft.setTextColor(Bluefruit.connected() ? ILI9341_GREEN : ILI9341_RED);
        tft.setCursor(225, 20);
        tft.print(Bluefruit.connected() ? "CONN" : "DISC");
      }
      
      modeDisplayVisible = true;
      lastModeUpdate = millis();
      lastDisplayedMode = currentMode;
    }
  } else if (modeDisplayVisible) {
    // Clear mode indicator
    tft.fillRect(220, 5, 95, 25, backgroundColor);
    modeDisplayVisible = false;
  }
}


void clearDisplayArea() {
  // erase the whole calculator window
  tft.fillRect(displayX + 1,
               displayY + 1,
               displayW - 2,
               displayH - 2,
               ILI9341_BLACK);

  // reset our “last displayed” state so the next update will draw from scratch
  lastDisplayCalc    = "";
  lastDisplayHistory = "";
  lastDisplayAlternate = "";

  // optionally, force a full redraw immediately
  displayNeedsUpdate = true;
}


                                                                      // --- drawInitialDisplay() ---
void drawInitialDisplay() {
  tft.fillScreen(backgroundColor); //          Background
  // outline the display
  tft.drawLine(displayX, displayY, displayW, displayY, ILI9341_WHITE);

  // *** Title Bar (use your 15-pt font) ***
  tft.setFont(&Arimo_VariableFont_wght7pt7b); //                                         FONT:title 
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(5, 20);
  tft.print("CALCULATOR");
  // back to default (bitmap) font
  tft.setFont();
}

                                                                                // --- updateDisplay() ---
void updateDisplay() {
  // 1) Main calculation line (use your 25-pt font)
  if (calcLine != lastDisplayCalc) {
    const int calcY = 160, calcH = 40;
    tft.fillRect(displayX + 2, calcY, displayW - 4, calcH, backgroundColor); //background

    if (calcLine.length()) {
      tft.setFont(&Arimo_VariableFont_wght15pt7b); //                                 FONT: Main
      tft.setTextSize(1);
      tft.setTextColor(ILI9341_WHITE);
      int16_t x, y;
      uint16_t w, h;
      tft.getTextBounds(calcLine, 0, 0, &x, &y, &w, &h);

      int cx = displayX + displayW - w - 8;
      if (cx < displayX + 2) cx = displayX + 2;
      int cy = calcY + (calcH + h)/2 - h/2;
      tft.setCursor(cx, cy);
      tft.print(calcLine);

      tft.setFont();
    }
    lastDisplayCalc = calcLine;
  }

  // 2) History line (use your 20-pt font)
  if (historyLine != lastDisplayHistory && historyLine.length()) {
    const int histY = 40, histH = 25;
    tft.fillRect(displayX + 2, histY, displayW - 4, histH, backgroundColor); //background

    tft.setFont(&Arimo_VariableFont_wght12pt7b); //                           FONT: History
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_RUBY);
    int16_t x, y;
    uint16_t w, h;
    tft.getTextBounds(historyLine, 0, 0, &x, &y, &w, &h);

    int cx = displayX + displayW - w - 8;
    if (cx < displayX + 2) cx = displayX + 2;
    tft.setCursor(cx, histY + h);
    tft.print(historyLine);

    tft.setFont();
    lastDisplayHistory = historyLine;
  }

  // 3) Alternate result (use your 12-pt font)
  if (alternateDisplay != lastDisplayAlternate && alternateDisplay.length()) {
    const int altY = 100, altH = 25;
    tft.fillRect(displayX + 2, altY, displayW - 4, altH, backgroundColor); //background

    tft.setFont(&Arimo_VariableFont_wght12pt7b); //                         FONT: Alternate 
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_DARKGREY);
    int16_t x, y;
    uint16_t w, h;
    tft.getTextBounds(alternateDisplay, 0, 0, &x, &y, &w, &h);

    int cx = displayX + displayW - w - 8;
    if (cx < displayX + 2) cx = displayX + 2;
    tft.setCursor(cx, altY + h);
    tft.print(alternateDisplay);

    tft.setFont();
    lastDisplayAlternate = alternateDisplay;
  }
}


void handleDisplay(){
  if (currentMode != MODE_CALCULATOR) return;
  static uint32_t lastUpdate = 0;
  uint32_t now = millis();
  if (displayNeedsUpdate && now - lastUpdate > 50){
    updateDisplay();
    displayNeedsUpdate = false;
    lastUpdate = now;
  }
}
