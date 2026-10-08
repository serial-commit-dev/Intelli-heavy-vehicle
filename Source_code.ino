#include <NewPing.h>

#define SONAR_NUM 3   // Left, Rear, Right
#define Max_Dist 200  // Maximum distance to ping (in cm)
#define PING_INTERVAL 33

// Pin Mapping: Rear(2,3), Right(4,5), Left(6,7)
NewPing sonar[SONAR_NUM] = {
  NewPing(6, 7, Max_Dist), // Index 0: Left Sensor
  NewPing(2, 3, Max_Dist), // Index 1: Rear Sensor
  NewPing(4, 5, Max_Dist)  // Index 2: Right Sensor
};

int green_indicator     = 8;
int left_red_indicator  = 9;
int right_red_indicator = 10;
int rear_red_indicator  = 11;

// Active Buzzer Pin
const int buzzer_pin = 12;

// Test Threshold set to 10 cm
const int obstacleDistance = 10;

int distances[SONAR_NUM] = {0, 0, 0};

unsigned long pingTimer = 0;
uint8_t currentSensor   = 0;

// Global timestamps for alert persistence
// Global timestamps for alert persistence
unsigned long left_millis     = 0; // Left sensor timer
unsigned long right_millis    = 0; // Right sensor timer
unsigned long rear_millis     = 0; // Rear sensor timer
unsigned long dual_millis     = 0; // Dual (Left+Right) timer
unsigned long Rg_re_millis    = 0; // Right+Rear timer
unsigned long Lf_re_millis    = 0; // Left+Rear timer
unsigned long code_red_millis = 0; // All blocked timer

const long alert_interval = 6000; // 6-second active alert window

// Global non-blocking blink clock
unsigned long blinkTimer = 0;
const long blink_interval = 250; // Flash/Beep rate (250ms ON / 250ms OFF)
bool blinkState = false;

// Non-blocking Serial print timer
unsigned long debugTimer = 0;
const long debug_interval = 500; // Print distances every 500ms

void setup() {
  Serial.begin(9600);
  Serial.println("=======================================================");
  Serial.println("Initializing 3 Ultrasonic Sensors (10cm Test Threshold)");
  Serial.println("=======================================================");

  pingTimer = millis();
 
  pinMode(green_indicator, OUTPUT);
  pinMode(left_red_indicator, OUTPUT);
  pinMode(right_red_indicator, OUTPUT);
  pinMode(rear_red_indicator, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  // Default state: Safe ON, hazards & buzzer OFF
  digitalWrite(green_indicator, HIGH);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(buzzer_pin, LOW);
}

void loop() {
  unsigned long current_millis = millis();

  // Non-blocking 250ms clock for LED flashing and active buzzer pulses
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
  printDebugInfo();
}

void processObstacleTasks() {
  // Triggers when object is closer than threshold (10 cm)
  bool leftBlocked  = (distances[0] > 0 && distances[0] < obstacleDistance);
  bool rearBlocked  = (distances[1] > 0 && distances[1] < obstacleDistance);
  bool rightBlocked = (distances[2] > 0 && distances[2] < obstacleDistance);

  unsigned long current_millis = millis();

  // 1. Stamp timestamps when specific obstacle combinations occur
  if (leftBlocked  && !rightBlocked && !rearBlocked) left_millis  = current_millis;
  if (!leftBlocked && rightBlocked  && !rearBlocked) right_millis = current_millis;
  if (!leftBlocked && !rightBlocked && rearBlocked)  rear_millis  = current_millis;
  
  if (leftBlocked  && rightBlocked  && !rearBlocked) dual_millis     = current_millis;
  if (!leftBlocked && rightBlocked  && rearBlocked)  Rg_re_millis    = current_millis;
  if (leftBlocked  && !rightBlocked && rearBlocked)  Lf_re_millis    = current_millis;
  if (leftBlocked  && rightBlocked  && rearBlocked)  code_red_millis = current_millis;

  // 2. Evaluate 6-second active alert windows (Highest priority first)
  if (code_red_millis > 0 && (current_millis - code_red_millis < alert_interval)) {
    code_red();
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
  else if (left_millis > 0 && (current_millis - left_millis < alert_interval)) {
    left_indicator();
  }
  else if (right_millis > 0 && (current_millis - right_millis < alert_interval)) {
    right_indicator();
  }
  else if (rear_millis > 0 && (current_millis - rear_millis < alert_interval)) {
    rear_indicator();
  }
  else {
    // All clear (> 10cm): Green LED ON, hazard LEDs OFF, Buzzer OFF
    digitalWrite(green_indicator, HIGH);
    digitalWrite(left_red_indicator, LOW);
    digitalWrite(right_red_indicator, LOW);
    digitalWrite(rear_red_indicator, LOW);
    digitalWrite(buzzer_pin, LOW);
  }
}
// ==========================================
// SERIAL MONITOR DEBUG FUNCTION
// ==========================================
void printDebugInfo() {
  unsigned long current_millis = millis();
  
  if (current_millis - debugTimer >= debug_interval) {
    debugTimer = current_millis;

    Serial.print("Left: ");
    Serial.print(distances[0]);
    Serial.print(" cm | Rear: ");
    Serial.print(distances[1]);
    Serial.print(" cm | Right: ");
    Serial.print(distances[2]);
    Serial.print(" cm");

    bool leftAlert  = (distances[0] > 0 && distances[0] < obstacleDistance);
    bool rearAlert  = (distances[1] > 0 && distances[1] < obstacleDistance);
    bool rightAlert = (distances[2] > 0 && distances[2] < obstacleDistance);

    if (leftAlert || rearAlert || rightAlert) {
      Serial.print(" ---> [ALERT] Threshold (<10cm) breached on: ");
      if (leftAlert)  Serial.print("LEFT ");
      if (rearAlert)  Serial.print("REAR ");
      if (rightAlert) Serial.print("RIGHT ");
    } else {
      Serial.print(" | Status: CLEAR");
    }
    
    Serial.println();
  }
}

// ==========================================
// INDICATOR FUNCTIONS
// ==========================================

void left_indicator() {                     // Indicating left side when breached on left side
  digitalWrite(green_indicator, LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  triggerBuzzer(1000);
}

void right_indicator() {                 //Indicating right side when breached on right side
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  triggerBuzzer(1000);
}

void rear_indicator() {
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
  triggerBuzzer(800);
}

void dual_indicator() {                         // Indicating both sides when breached on both sides
  digitalWrite(green_indicator, LOW);
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  triggerBuzzer(1500);
}

void Lf_re_indicator() {                        // Indicating Left and Rear side when breached on both of those sides
  digitalWrite(green_indicator, LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
  triggerBuzzer(1500);
}

void Rg_re_indicator() {                        // Indicating Right and Rear side when breached on both of those sides
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
  triggerBuzzer(1500);
}

void code_red() {                           //Indicating that the truck hs been breached from all three sides
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
  triggerBuzzer(2000);
}

// ==========================================
// BUZZER TRIGGER (NO TIMER CONFLICT)
// ==========================================
void triggerBuzzer(unsigned int pitchFrequency) {
  // Toggles buzzer ON/OFF with blinkState (250ms ON / 250ms OFF)
  digitalWrite(buzzer_pin, blinkState ? HIGH : LOW);
}
