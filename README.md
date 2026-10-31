# Autonomous Maze Navigation Robot

Arduino C++ control software used by our university team to complete a maze with a physical robot. The robot combines **3 infrared sensing channels** and **2 continuous-rotation servos** with wall following, corridor steering, and repeated-route detection.

## Project highlights

- **Physical maze completion:** our team completed the maze using this sketch, as confirmed by Ailinna.
- **32-action rolling history:** records turns and retains the most recent decisions.
- **Repeated-route detection:** compares repeating turn patterns of lengths **2 to 8**, then switches the wall-following preference.
- **Adaptive steering:** adjusts wheel pulses using two-wall, left-wall, right-wall, or open-space feedback.
- **Junction handling:** combines right-priority exploration, recovery turns, and post-turn alignment.
- **Terminal stop:** stops both motors at a blocked end after the configured long-corridor interval.

## Hardware configuration

| Component | Pin |
| --- | --- |
| Left / centre / right IR LEDs | 10 / 6 / 2 |
| Left / centre / right IR receivers | 11 / 7 / 3 |
| Left / centre / right indicators | A2 / A1 / A0 |
| Left / right servo signal | 13 / 12 |

Open `maze_robot/maze_robot.ino` in Arduino IDE with the Servo library and select the board used by your robot.

The sketch uses channel-specific IR frequency sweeps and calibrated servo pulses. Servo neutral is 1500 microseconds; turn durations, sensing thresholds, and drive pulses should be calibrated for your hardware. Start with the wheels clear of the ground.

## Navigation flow

1. Sweep the three infrared channels and update the wall indicators.
2. Use corridor duration to distinguish a terminal stop from a dead-end turnaround.
3. Choose a turn using the current wall-following preference and available openings.
4. Record completed left/right turns and check for repeated patterns.
5. Switch the wall-following preference when a repeated pattern is detected.
6. Adjust steering from the available wall readings and realign after turns.

## Automated checks

```bash
c++ -std=c++17 -Wall -Wextra -pedantic -I tests -include tests/Arduino.h tests/test_passed_control.cpp -o /tmp/maze-control-test
/tmp/maze-control-test
```

The host-side suite covers **6 groups of control checks**: infrared sweeps, history retention, loop recovery, steering modes, turn selection, and terminal stopping. GitHub Actions compiles and runs the suite on pushes and pull requests.

The tests exercise the supplied sketch with simulated Arduino inputs. The physical maze completion is the team's hardware result, confirmed by Ailinna. See [validation and source notes](docs/VALIDATION.md) for the distinction.

## Source

The published sketch comes from `project_text.txt`, supplied and identified by Ailinna as the version used for the successful physical maze run. Its control logic is preserved; the import normalises line endings and trailing whitespace. This is a university team project presented in Ailinna Zhang's portfolio.
