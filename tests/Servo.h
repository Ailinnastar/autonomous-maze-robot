#pragma once

class Servo {
public:
    int pin = -1;
    int lastPulse = 0;
    void attach(int value) { pin = value; }
    void writeMicroseconds(int value) { lastPulse = value; }
};
