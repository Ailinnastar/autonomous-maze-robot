# Autonomous Maze Navigation Robot

Arduino C++ control software using three infrared sensor channels and two continuous-rotation servos. The recovered sketch demonstrates right-priority intersection handling, timed turns, corridor steering, dead-end turns, and a terminal-stop heuristic.

## Source provenance

This sketch was recovered from Ailinna's editor history for the original `PROJECT_CODE_UNREFINED.ino` project. It is a May 2026 development baseline, not a verified copy of the final submission. The source labels essential motion and sensing routines as a lab baseline; that educational attribution is retained. Do not infer sole authorship of all baseline routines.

The CV describes additional cycle-recovery behaviour and a maze completion time. Those claims have not been reproduced with this recovered snapshot and are not asserted for this repository.

## Hardware configuration

| Component | Pin |
| --- | --- |
| Left / centre / right IR LEDs | 10 / 6 / 2 |
| Left / centre / right IR receivers | 11 / 7 / 3 |
| Left / centre / right indicators | A2 / A1 / A0 |
| Left / right servo signal | 13 / 12 |

Open `maze_robot/maze_robot.ino` in Arduino IDE with the Servo library. Select the actual board used by your hardware before compiling. The repository does not establish a verified board target or wiring specification beyond the pin constants in the recovered code.

Servo neutral is configured at 1500 microseconds. IR thresholds, drive pulses, turn durations, and steering trim are hardware-dependent and require calibration. Test with the wheels clear of the ground before allowing movement.

## Control flow

1. Read three infrared channels and update indicators.
2. At a dead end, perform a timed turn or stop according to the corridor-time heuristic.
3. Otherwise prioritise an open right branch, then a left turn if the front is blocked.
4. Use right-sensor feedback to steer in a corridor.

The terminal heuristic stops after a dead-end observation following a period without a side opening. It does not recognise a marked finish line or guarantee that an arbitrary maze has been solved.

## Host-side checks

```bash
c++ -std=c++17 -Wall -Wextra -pedantic -I tests -include tests/Arduino.h tests/test_control.cpp -o /tmp/maze-control-test
/tmp/maze-control-test
```

These tests compile the recovered sketch against small Arduino/Servo stubs and check decision branches, motor stops, and terminal behaviour. They do not emulate IR physics, verify electrical compatibility, compile for an AVR target, or validate real-world navigation. No hardware performance benchmark is claimed.
