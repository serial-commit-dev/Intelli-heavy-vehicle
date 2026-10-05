#include <NewPing.h>

#define SONAR_NUM 3   // number of sensors
#define Max_Dist 200
#define PING_INTERVAL 33

NewPing sonar[SONAR_NUM] = {
  NewPing(2, 3, Max_Dist), // Sensor 1: Left
  NewPing(4, 5, Max_Dist), // Sensor 2: Center/Rear
  NewPing(6, 7, Max_Dist)  // Sensor 3: Right
};

int green_indicator     = 8;
int left_red_indicator  = 9;
int right_red_indicator = 10;
int rear_red_indicator  = 11;

int distances[SONAR_NUM] = {0, 0, 0};

unsigned long pingTimer = 0;
uint8_t currentSensor   = 0;

// Global timestamps for alert persistence
unsigned long previous_millis        = 0; // Left timer
unsigned long left_millis            = 0; // Right timer
unsigned long rear_millis            = 0; // Rear timer
unsigned long dual_millis            = 0; // Dual (Left+Right) timer
unsigned long Rg_re_millis           = 0; // Right+Rear timer
unsigned long Lf_re_millis           = 0; // Left+Rear timer
unsigned long code_red_millis        = 0; // All blocked timer

const long alert_interval = 6000; // 6-second active alert window

// Global non-blocking blink clock
unsigned long blinkTimer = 0;
const long blink_interval = 250; // Flash rate (250ms ON / 250ms OFF)
bool blinkState = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing 3 Ultrasonic Sensors...");

  pingTimer = millis(); // Initializing Start timer

  pinMode(green_indicator, OUTPUT);
  pinMode(left_red_indicator, OUTPUT);
  pinMode(right_red_indicator, OUTPUT);
  pinMode(rear_red_indicator, OUTPUT);

  // Default state: Safe ON, hazards OFF
  digitalWrite(green_indicator, HIGH);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(rear_red_indicator, LOW);
}

void loop() {
  unsigned long current_millis = millis();

  // Non-blocking 250ms clock for smooth LED flashing
  if (current_millis - blinkTimer >= blink_interval) {
    blinkTimer = current_millis;
    blinkState = !blinkState;
  }

  // Interleaved non-blocking sonar pings
  if (current_millis - pingTimer >= PING_INTERVAL) {
    pingTimer = current_millis;

    int reading = sonar[currentSensor].ping_cm();
    distances[currentSensor] = (reading == 0) ? Max_Dist : reading;

    currentSensor++;
    if (currentSensor >= SONAR_NUM) {
      currentSensor = 0;
    }
  }

  processObstacleTasks();
}

void processObstacleTasks() {
  bool leftBlocked  = (distances[0] < 200);
  bool rearBlocked  = (distances[1] < 200);
  bool rightBlocked = (distances[2] < 200);

  unsigned long current_millis = millis();

  // Stamping timestamps when sensors detect obstacles
  if (leftBlocked && !rightBlocked && !rearBlocked) previous_millis = current_millis;
  if (!leftBlocked && rightBlocked && !rearBlocked)  left_millis     = current_millis;
  if (!leftBlocked && !rightBlocked && rearBlocked)  rear_millis     = current_millis;
  if (leftBlocked && rightBlocked && !rearBlocked)   dual_millis     = current_millis;
  if (!leftBlocked && rightBlocked && rearBlocked)   Rg_re_millis    = current_millis;
  if (leftBlocked && !rightBlocked && rearBlocked)   Lf_re_millis    = current_millis;
  if (leftBlocked && rightBlocked && rearBlocked)    code_red_millis = current_millis;

  // Check 6-second window (< alert_interval)
  if (previous_millis > 0 && (current_millis - previous_millis < alert_interval)) {
    left_indicator();
  }
  else if (left_millis > 0 && (current_millis - left_millis < alert_interval)) {
    right_indicator();
  }
  else if (rear_millis > 0 && (current_millis - rear_millis < alert_interval)) {
    rear_indicator();
  }
  else if (dual_millis > 0 && (current_millis - dual_millis < alert_interval)) {
    dual_indicator();
  }
  else if (Rg_re_millis > 0 && (current_millis - Rg_re_millis < alert_interval)) {
    Rg_re_indicator();
  }
  else if (Lf_re_millis > 0 && (current_millis - Lf_re_millis < alert_interval)) {
    Lf_re_indicator();
  }
  else if (code_red_millis > 0 && (current_millis - code_red_millis < alert_interval)) {
    code_red();
  }
  else {
    // All clear: Green LED ON, all hazard LEDs OFF
    digitalWrite(green_indicator, HIGH);
    digitalWrite(left_red_indicator, LOW);
    digitalWrite(right_red_indicator, LOW);
    digitalWrite(rear_red_indicator, LOW);
  }
}

// ==========================================
// YOUR INDICATOR FUNCTIONS (Non-Blocking)
// ==========================================

void left_indicator() {
  digitalWrite(green_indicator, LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
}

void right_indicator() {
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
}

void rear_indicator() {
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
}

void dual_indicator() {
  digitalWrite(green_indicator, LOW);
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
}

void code_red() {
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
}

void Lf_re_indicator() {
  digitalWrite(green_indicator, LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
}

void Rg_re_indicator() {
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
}
