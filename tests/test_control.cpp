#include <cassert>
#include <iostream>
#include "../maze_robot/maze_robot.ino"

void resetSensors(int left, int centre, int right) {
    fakeTimeMs = 5000;
    lastTimeLeftOrRightOpenMs = fakeTimeMs;
    finishReached = false;
    receiverValues[irReceiverPinLeft] = left;
    receiverValues[irReceiverPinMid] = centre;
    receiverValues[irReceiverPinRight] = right;
}

int main() {
    setup();
    assert(servoLeft.pin == 13 && servoRight.pin == 12);

    resetSensors(0, 1, 1);
    loop();
    assert(!finishReached);
    assert(servoLeft.lastPulse == MED_CCW && servoRight.lastPulse == MED_CW);
    assert(fakeTimeMs == 5000 + PAUSE_TIME + TURN_TIME + PAUSE_TIME + 10);

    resetSensors(1, 0, 0);
    loop();
    assert(servoLeft.lastPulse == MED_CCW && servoRight.lastPulse == MED_CW);

    resetSensors(0, 0, 0);
    loop();
    assert(!finishReached);
    assert(fakeTimeMs == 5000 + PAUSE_TIME + UTURN_TIME + PAUSE_TIME + 10);
    assert(servoLeft.lastPulse == STOP && servoRight.lastPulse == STOP);

    resetSensors(0, 0, 0);
    fakeTimeMs += LONG_END_NO_SIDE_OPEN_MS;
    loop();
    assert(finishReached);
    assert(servoLeft.lastPulse == STOP && servoRight.lastPulse == STOP);
    auto stoppedAt = fakeTimeMs;
    loop();
    assert(fakeTimeMs == stoppedAt);
    std::cout << "Host control checks passed; physical hardware was not tested.\n";
}
