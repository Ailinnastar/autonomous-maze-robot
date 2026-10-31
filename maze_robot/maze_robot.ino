#include <Servo.h>                      // Include servo library

Servo servoLeft;   // left wheel
Servo servoRight;   // right wheel
int LS = 1600;            // left speed
int RS= 1400;            // right speed
bool finished = false;
unsigned long corridor_start_time = 0;
bool in_corridor = false;
unsigned long corridor_duration;
bool follow_left = false;
int last_d_R = 0;
int last_d_L = 0;
const int action_max = 32;
const int max_pattern_length = 8;
char actions[action_max];
int action_count = 0;
const unsigned long long_dead_end = 3000; //CALIBRATE THIS LATER

const int irLedPin_L= 10, irReceiverPin_L=11;
const int redLedPin_L = A2;

const int irLedPin_M= 6, irReceiverPin_M=7;
const int redLedPin_M = A1;

const int irLedPin_R= 2, irReceiverPin_R=3;
const int redLedPin_R = A0;



long low_freq_L = 38000;
long low_freq_M = 36000;
long low_freq_R = 34000;

long high_freq_L = 42000;
long high_freq_M = 40000;
long high_freq_R = 42000;

int step_L = 1000;
int step_M = 1000;
int step_R = 2000;



void setup()
{
    Serial.begin(9600);
    servoLeft.attach(13);
    servoRight.attach(12);

    pinMode(irReceiverPin_L, INPUT);            // IR left receiver pin
    pinMode(irReceiverPin_M, INPUT);            // IR middle receiver pin
    pinMode(irReceiverPin_R, INPUT);            // IR right receiver pin

    pinMode(irLedPin_L, OUTPUT);                // IR left LED pin
    pinMode(irLedPin_M, OUTPUT);                // IR middle LED pin
    pinMode(irLedPin_R, OUTPUT);                // IR right LED pin

    pinMode(redLedPin_L, OUTPUT);               // Red left LED pin
    pinMode(redLedPin_M, OUTPUT);               // Red middle LED pin
    pinMode(redLedPin_R, OUTPUT);               // Red right LED pin

}

void loop()    {
  if (finished) {
    return;
  }

  int dist_L = irDistance(irLedPin_L, irReceiverPin_L, low_freq_L, high_freq_L, step_L);
  int dist_R = irDistance(irLedPin_R, irReceiverPin_R, low_freq_R, high_freq_R, step_R);
  int dist_M  = irDistance(irLedPin_M, irReceiverPin_M, low_freq_M, high_freq_M, step_M);

  bool wall_left = (dist_L <= 4);
  bool wall_right = (dist_R <= 4);

  bool wall_front = (dist_M < 4);
  if (wall_left) {
    digitalWrite(redLedPin_L, HIGH);
  } else {
    digitalWrite(redLedPin_L, LOW);
  }
  if (wall_right) {
    digitalWrite(redLedPin_R, HIGH);
  } else {
    digitalWrite(redLedPin_R, LOW);
  }
  if (wall_front) {
    digitalWrite(redLedPin_M, HIGH);
  } else{
    digitalWrite(redLedPin_M, LOW);
  }
  Serial.println("M");
  Serial.println(dist_M);
  Serial.println("L");
  Serial.println(dist_L);
  Serial.println("R");
  Serial.println(dist_R);


  if (wall_front) {
    if (wall_left && wall_right) {
      if (in_corridor) {
        corridor_duration = millis() - corridor_start_time;
      } else {
        corridor_duration = 0;
      }
      if (corridor_duration > long_dead_end){
        stop();
        return;
      } else {
        uTurn();
      }
    } else if (wall_left) {
      turnRight();
    } else if (wall_right) {
      turnLeft();
    } else {
      if (follow_left) {
        turnLeft(); //Will turn left if it is found to be in a loop
        follow_left = false;
      } else {
        turnRight(); //Normally following the right wall
      }
    }
    check_for_loop();
    in_corridor = false;
  } else if ((follow_left && !wall_left) || (!follow_left && !wall_right)) {
    if (follow_left) {
      turnLeft();
      follow_left = false;
    } else {
      turnRight();
    }
    check_for_loop();
    in_corridor = false;
  } else {
    if (wall_left && wall_right){
      if (!in_corridor) {
        corridor_start_time = millis();
        in_corridor = true;
      }
    } else {
      in_corridor = false;
    }
    driveStraight(dist_L, dist_R);
  }


}






void log_action(char f) {
  if (action_count < action_max) { //adds actions to the list
    actions[action_count++] = f;
  } else {
    for (int i = 0; i < action_max - 1; i++) { //drop the oldest action
      actions[i] = actions[i + 1];
    }
    actions[action_max - 1] = f;
  }
}

bool detect_loop() {
  for (int n = 2; n <= max_pattern_length; n++) { //n represents the pattern size for example in (RLLRLL) the pattern size is 3
    if (action_count < 2 * n) continue; //need to verify that the action count (or size of the actions list) is long enough have a repeat of a pattern of size n
    bool match = true;
    for (int i = 0; i < n; i++) {
      if (actions[action_count - 1 - i] != actions[action_count - 1 - n - i]) {
        match = false;
        break;
      }
    }
    if (match) return true;
  }
  return false;
}


void check_for_loop() {
  if (detect_loop()) {
    follow_left = !follow_left;
    action_count = 0;   // reset so we don't immediately retrigger
  }
}

void driveStraight(int d_L, int d_R){ //main drive straight function
    if (d_L < 5 && d_R < 5) {
          // Two-wall mode — recalibrate
        last_d_L = d_L;
        last_d_R = d_R;
        int diff = (d_R - d_L);
        LS = 1550 + 10 * diff;
        RS = 1450 + 10 * diff;

    } else if (d_L < 5) {
        // Left-only: aim for the same left-wall distance we had when centered
        int diff = 1 - d_L;     // negative if too close, positive if too far
        LS = 1550 + 10 * diff;
        RS = 1450 + 10 * diff;
        digitalWrite(redLedPin_L, HIGH);


    } else if (d_R < 5) {
        // Right-only: aim for the same right-wall distance we had when centered
        int diff = d_R - 2;     // negative if too close, positive if too far
        LS = 1550 + 10 * diff;
        RS = 1450 + 10 * diff;
        digitalWrite(redLedPin_R, HIGH);

    } else {
        // No walls visible, or no calibration yet — neutral
        LS = 1580;
        RS = 1400;
        digitalWrite(redLedPin_R, LOW);
        digitalWrite(redLedPin_L, LOW);
    }
    servoLeft.writeMicroseconds(LS);
    servoRight.writeMicroseconds(RS);
}

void driveStraightV2(int d_L, int d_R){ // used when the robot is leaving a turn, this one never adjusts using one wall
    if (d_L < 5 && d_R < 5) {
        // Two-wall mode: remember these values for later single-wall use
        last_d_L = d_L;
        last_d_R = d_R;

        int diff = (d_R - d_L);
        LS = 1550 + 10 * diff;
        RS = 1450 + 10 * diff;
    } else {
        LS = 1580;
        RS = 1400;
        digitalWrite(redLedPin_R, LOW);
        digitalWrite(redLedPin_L, LOW);
    }
    servoLeft.writeMicroseconds(LS);
    servoRight.writeMicroseconds(RS);
}
bool driveUntilFrontWall(unsigned long timeout) { // executes when the turn is first registered
    unsigned long start = millis();
    while (millis() - start < timeout) {
        int d_L = irDistance(irLedPin_L, irReceiverPin_L, low_freq_L, high_freq_L, step_L);
        int d_R = irDistance(irLedPin_R, irReceiverPin_R, low_freq_R, high_freq_R, step_R);
        int d_M = irDistance(irLedPin_M, irReceiverPin_M, low_freq_M, high_freq_M, step_M);
        if (d_L <= 4 && d_R <= 4) { //aborts whenever it registers two walls by its side
          digitalWrite(redLedPin_L, HIGH);
          digitalWrite(redLedPin_R, HIGH);
          return false;
        }
        if (d_M < 4) {
          digitalWrite(redLedPin_M, HIGH);
          return true;
        }


        driveStraight(d_L, d_R);
    }
    return true;
}
void Adjustoutofturn(unsigned long timeout) {
    unsigned long start = millis();
    while (millis() - start < timeout) {
        int d_L = irDistance(irLedPin_L, irReceiverPin_L, low_freq_L, high_freq_L, step_L);
        int d_R = irDistance(irLedPin_R, irReceiverPin_R, low_freq_R, high_freq_R, step_R);

        if (d_L <= 4 && d_R <= 4) return;
        driveStraightV2(d_L, d_R);
    }
}
void turnLeft() {
  Serial.println("Left turn?");

  if (!driveUntilFrontWall(700)) {
    return;
  }
  log_action('L');
  Serial.println("Left turn");
  servoLeft.writeMicroseconds(1499);
  servoRight.writeMicroseconds(1500);
  delay(750);
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1400);
  delay(1080);
  servoLeft.writeMicroseconds(1499);
  servoRight.writeMicroseconds(1500);
  delay(600);
  Adjustoutofturn(700);
}

void turnRight() {
  Serial.println("Right turn?");
  if (!driveUntilFrontWall(700)) {
    return;
  }
  log_action('R');
  Serial.println("Right turn");
  servoLeft.writeMicroseconds(1499);
  servoRight.writeMicroseconds(1500);
  delay(750);
  servoLeft.writeMicroseconds(1600);
  servoRight.writeMicroseconds(1500);
  delay(1110);
  servoLeft.writeMicroseconds(1499);
  servoRight.writeMicroseconds(1500);
  delay(750);
  Adjustoutofturn(700);
}

void uTurn() {
  Serial.println("U turn");
  servoLeft.writeMicroseconds(1499);
  servoRight.writeMicroseconds(1500);
  delay(750);
  servoLeft.writeMicroseconds(1600);
  servoRight.writeMicroseconds(1600);
  delay(1230);
  servoLeft.writeMicroseconds(1499);
  servoRight.writeMicroseconds(1500);
  delay(750);


}

void stop(){
  servoLeft.writeMicroseconds(1500);
  servoRight.writeMicroseconds(1500);
  finished = true;
}


// IR Object Detection Function

int irDetect(int irLedPin, int irReceiverPin, long frequency)
{
  tone(irLedPin, frequency);                 // Turn on the IR LED square wave
  delay(1);                                  // Wait 1 ms
  int ir = digitalRead(irReceiverPin);       // IR receiver -> ir variable
  noTone(irLedPin);                          // Turn off the IR LED
  delay(1);                                  // Down time before recheck
  return ir;                                 // Return 0 detect, 1 no detect
}

int irDistance(int irLedPin, int irReceiverPin, long low_freq, long high_freq, int step)
{
   int distance = 0;
   for(long f = low_freq; f <= high_freq; f += step)
   {
      distance += irDetect(irLedPin, irReceiverPin, f);
   }
   return distance;
}
