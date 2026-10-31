#pragma once
#include <cstdlib>

constexpr int A0 = 14, A1 = 15, A2 = 16;
constexpr int INPUT = 0, OUTPUT = 1, LOW = 0, HIGH = 1;
inline unsigned long fakeTimeMs = 0;
inline int receiverValues[32] = {};
inline int receiverOffsets[32] = {};
inline int receiverReads[32] = {};
inline void delay(unsigned long ms) { fakeTimeMs += ms; }
inline void delayMicroseconds(unsigned int) {}
inline unsigned long millis() { return fakeTimeMs; }
inline void pinMode(int, int) {}
inline void tone(int, long) {}
inline void noTone(int) {}
inline int digitalRead(int pin) {
    ++receiverReads[pin];
    return receiverOffsets[pin]++ % 5 < receiverValues[pin] ? HIGH : LOW;
}
inline void digitalWrite(int, int) {}

struct SerialStub {
    void begin(long) {}
    template <typename T> void println(const T&) {}
};
inline SerialStub Serial;
