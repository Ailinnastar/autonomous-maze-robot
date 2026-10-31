#pragma once
#include <cstdlib>

constexpr int A0 = 14, A1 = 15, A2 = 16;
constexpr int INPUT = 0, OUTPUT = 1, LOW = 0, HIGH = 1;
inline unsigned long fakeTimeMs = 0;
inline int receiverValues[32] = {};
inline void delay(unsigned long ms) { fakeTimeMs += ms; }
inline void delayMicroseconds(unsigned int) {}
inline unsigned long millis() { return fakeTimeMs; }
inline void pinMode(int, int) {}
inline void tone(int, long) {}
inline void noTone(int) {}
inline int digitalRead(int pin) { return receiverValues[pin]; }
inline void digitalWrite(int, int) {}
