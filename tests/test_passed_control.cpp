#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

// The Arduino build generates these declarations before compiling a sketch.
int irDetect(int, int, long);
int irDistance(int, int, long, long, int);
void log_action(char);
bool detect_loop();
void check_for_loop();
void driveStraight(int, int);
void driveStraightV2(int, int);
bool driveUntilFrontWall(unsigned long);
void Adjustoutofturn(unsigned long);
void turnLeft();
void turnRight();
void uTurn();
void stop();

#include "../maze_robot/maze_robot.ino"

void resetRobot(int left = 0, int centre = 0, int right = 0) {
    fakeTimeMs = 100;
    std::fill(std::begin(receiverValues), std::end(receiverValues), 0);
    std::fill(std::begin(receiverOffsets), std::end(receiverOffsets), 0);
    std::fill(std::begin(receiverReads), std::end(receiverReads), 0);
    receiverValues[irReceiverPin_L] = left;
    receiverValues[irReceiverPin_M] = centre;
    receiverValues[irReceiverPin_R] = right;
    finished = false;
    in_corridor = false;
    corridor_start_time = 0;
    corridor_duration = 0;
    follow_left = false;
    action_count = 0;
    std::fill(std::begin(actions), std::end(actions), '\0');
    LS = 1600;
    RS = 1400;
    last_d_L = last_d_R = 0;
    servoLeft.lastPulse = servoRight.lastPulse = 0;
}

void record(const std::string& sequence) {
    for (char action : sequence) log_action(action);
}

void checkSensorSweeps() {
    resetRobot(2, 4, 3);
    assert(irDistance(irLedPin_L, irReceiverPin_L, 38000, 42000, 1000) == 2);
    assert(irDistance(irLedPin_M, irReceiverPin_M, 36000, 40000, 1000) == 4);
    assert(irDistance(irLedPin_R, irReceiverPin_R, 34000, 42000, 2000) == 3);
    assert(receiverReads[irReceiverPin_L] == 5);
    assert(receiverReads[irReceiverPin_M] == 5);
    assert(receiverReads[irReceiverPin_R] == 5);
}

void checkHistoryWindow() {
    resetRobot();
    for (int i = 0; i < 34; ++i) log_action('0' + i % 10);
    assert(action_count == 32);
    assert(actions[0] == '2');
    assert(actions[31] == '3');
}

void checkLoopRecovery() {
    resetRobot();
    assert(!detect_loop());
    record("RLL");
    assert(!detect_loop());
    record("RLL");
    assert(detect_loop());
    check_for_loop();
    assert(follow_left && action_count == 0);
    assert(!detect_loop());
    record("LRLR");
    check_for_loop();
    assert(!follow_left && action_count == 0);

    record("RLLRRLRLRLLRRLRL");
    assert(detect_loop());
    actions[15] = 'U';
    assert(!detect_loop());
}

void checkSteeringModes() {
    resetRobot();
    driveStraight(2, 4);
    assert(servoLeft.lastPulse == 1570 && servoRight.lastPulse == 1470);
    driveStraight(2, 5);
    assert(servoLeft.lastPulse == 1540 && servoRight.lastPulse == 1440);
    driveStraight(5, 3);
    assert(servoLeft.lastPulse == 1560 && servoRight.lastPulse == 1460);
    driveStraight(5, 5);
    assert(servoLeft.lastPulse == 1580 && servoRight.lastPulse == 1400);
    driveStraightV2(2, 4);
    assert(servoLeft.lastPulse == 1570 && servoRight.lastPulse == 1470);
    driveStraightV2(2, 5);
    assert(servoLeft.lastPulse == 1580 && servoRight.lastPulse == 1400);
}

void checkTurnSelection() {
    resetRobot(0, 5, 5);
    loop();
    assert(action_count == 1 && actions[0] == 'R');
    assert(!finished);

    resetRobot(5, 0, 0);
    loop();
    assert(action_count == 1 && actions[0] == 'L');

    resetRobot(5, 5, 5);
    follow_left = true;
    loop();
    assert(action_count == 1 && actions[0] == 'L');
    assert(!follow_left);

    resetRobot(0, 5, 0);
    turnRight();
    assert(action_count == 0);
}

void checkDeadEndAndFinish() {
    resetRobot();
    loop();
    assert(!finished);
    assert(fakeTimeMs > 2000);

    resetRobot(0, 5, 0);
    loop();
    assert(in_corridor && !finished);
    fakeTimeMs = corridor_start_time + 3001;
    receiverValues[irReceiverPin_M] = 0;
    loop();
    assert(finished);
    assert(servoLeft.lastPulse == 1500 && servoRight.lastPulse == 1500);
    const auto stoppedAt = fakeTimeMs;
    loop();
    assert(fakeTimeMs == stoppedAt);
}

int main() {
    setup();
    assert(servoLeft.pin == 13 && servoRight.pin == 12);
    checkSensorSweeps();
    checkHistoryWindow();
    checkLoopRecovery();
    checkSteeringModes();
    checkTurnSelection();
    checkDeadEndAndFinish();
    std::cout << "Passed-version host checks: sensing, history, loop recovery, "
                 "steering, turns, and terminal stop.\n";
}
