// ---------- PIN DEFINITIONS ----------
#define Trig 12
#define Echo 13

#define ENA 5
#define ENB 6
#define IN1 3
#define IN2 4
#define IN3 2
#define IN4 7

// ---- IR Sensors ----
#define IR_LEFT_OUTER   A3
#define IR_LEFT_INNER   A2
#define IR_RIGHT_INNER  A1
#define IR_RIGHT_OUTER  A0

// ---------- CONTROL FLAGS ----------
volatile bool piStopOverride = false;

// ---------- DISTANCE ----------
const int stopDistance = 15;

// ---------- TIMING ----------
const unsigned long LOOP_INTERVAL = 40;
unsigned long lastLoopTime = 0;

// ---------- SERIAL BUFFER ----------
char serialBuf[16];
byte serialIdx = 0;

// ---------- SETUP ----------
void setup() {
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(IR_LEFT_OUTER, INPUT);
  pinMode(IR_LEFT_INNER, INPUT);
  pinMode(IR_RIGHT_INNER, INPUT);
  pinMode(IR_RIGHT_OUTER, INPUT);

  Serial.begin(9600);
  Serial.println("===== ARDUINO READY =====");
}

// ---------- ULTRASONIC ----------
long getDistance() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  long duration = pulseIn(Echo, HIGH, 15000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

// ---------- MOTOR CONTROL ----------
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void moveForward(int l, int r) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, l);
  analogWrite(ENB, r);
}

void spinLeft(int s) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, s);
  analogWrite(ENB, s);
}

void spinRight(int s) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, s);
  analogWrite(ENB, s);
}

// ---------- READ PI COMMAND (ROBUST) ----------
void readPiCommand() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      serialBuf[serialIdx] = '\0';
      serialIdx = 0;

      if (strcasecmp(serialBuf, "STOP") == 0) {
        piStopOverride = true;
        Serial.println(">> PI COMMAND: STOP");
      }
      else if (strcasecmp(serialBuf, "GO") == 0) {
        piStopOverride = false;
        Serial.println(">> PI COMMAND: GO");
      }
    }
    else if (serialIdx < sizeof(serialBuf) - 1) {
      serialBuf[serialIdx++] = c;
    }
  }
}

// ---------- MAIN LOOP ----------
void loop() {

  if (millis() - lastLoopTime < LOOP_INTERVAL) return;
  lastLoopTime = millis();

  readPiCommand();

  long distance = getDistance();

  // ---------- PRIORITY 1: PI STOP ----------
  if (piStopOverride) {
    stopMotors();
    return;
  }

  // ---------- PRIORITY 2: ULTRASONIC ----------
  if (distance <= stopDistance) {
    stopMotors();
    return;
  }

  // ---------- LANE FOLLOW ----------
  int L_out = digitalRead(IR_LEFT_OUTER);
  int L_in  = digitalRead(IR_LEFT_INNER);
  int R_in  = digitalRead(IR_RIGHT_INNER);
  int R_out = digitalRead(IR_RIGHT_OUTER);

  if (L_out == LOW) {
    spinLeft(110);
  }
  else if (L_in == LOW) {
    moveForward(75, 130);
  }
  else if (R_out == LOW) {
    spinRight(110);
  }
  else if (R_in == LOW) {
    moveForward(130, 75);
  }
  else {
    moveForward(75, 75);
  }
}
