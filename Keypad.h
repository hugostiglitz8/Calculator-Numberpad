//Keypad.h

#ifndef KEYPAD_H
#define KEYPAD_H

#include <Arduino.h>

// ── Modes ──
enum Mode { MODE_CALCULATOR, MODE_NUMBERPAD };
extern Mode currentMode;

// ── Keypad matrix pins ──
extern const uint8_t colPins[5];
extern const uint8_t rowPins[5];

// ── Key labels ──
extern const char* keymap[5][5];

// ── Key event buffering ──
#define KEY_BUFFER_SIZE 10
struct KeyEvent { uint8_t row, col; };
extern volatile KeyEvent   keyBuffer[KEY_BUFFER_SIZE];
extern volatile uint8_t    keyHead, keyTail;

// ── Zero-hold state ──
extern unsigned long  zeroPressStart;
extern bool           zeroPressed;
extern bool           zeroHoldActive;
extern const unsigned long zeroHoldThreshold;

// ── Debounce timing ──
extern volatile uint32_t lastKeyTime[5][5];
extern const uint32_t    keyDebounceMs;

// ── Functions ──
void initKeypad();
void keyScan();
void detectZeroHold();
void processKeyBuffer();

#endif // KEYPAD_H