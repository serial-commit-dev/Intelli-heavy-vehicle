#include <NewPing.h>

#define SONAR_NUM 3   // Number of sensors
#define Max_Dist 400
#define PING_INTERVAL 33
#define OBSTACLE_THRESHOLD 200 // Standard obstacle detection threshold in cm

// Fixed pin assignments: Sensor 3 changed from (6,7) to (6,13) to avoid conflict with green_indicator on Pin 8
NewPing sonar[SONAR_NUM] = {
  NewPing(2, 3, Max_Dist), // Sensor 0: Left
  NewPing(4, 5, Max_Dist), // Sensor 1: Rear / Center
  NewPing(6, 13, Max_Dist) // Sensor 2: Right
};

int green_indicator = 8;
int left_red_indicator = 9;
int right_red_indicator = 10;
int rear_red_indicator = 11;
int buzzer = 12;

int distances[SONAR_NUM] = {0, 0, 0};

unsigned long pingTimer = 0;
uint8_t currentSensor = 0;

// Non-blocking LED timing variables
unsigned long blinkTimer = 0;
const unsigned long BLINK_INTERVAL = 250; // 250ms toggle for visible blinking
bool blinkState = false;


//////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long = previous_millis = 0; //Estimated time for to take-off
const long intervals = 12000;



void setup() {
  Serial.begin(9600);
  Serial.println("Initializing 3 Ultrasonic Sensors...");

  pingTimer = millis();

  pinMode(green_indicator, OUTPUT);
  pinMode(left_red_indicator, OUTPUT);
  pinMode(right_red_indicator, OUTPUT);
  pinMode(rear_red_indicator, OUTPUT);
  pinMode(buzzer, OUTPUT);
}

void loop() {
  // Non-blocking ping interval check
  if (millis() - pingTimer >= PING_INTERVAL) {
    pingTimer = millis();

    int reading = sonar[currentSensor].ping_cm();
    distances[currentSensor] = (reading == 0) ? Max_Dist : reading;

    currentSensor++;
    if (currentSensor >= SONAR_NUM) {
      currentSensor = 0;
    }
  }

  // Non-blocking blink clock for indicators
  if (millis() - blinkTimer >= BLINK_INTERVAL) {
    blinkTimer = millis();
    blinkState = !blinkState;
  }
  
  processObstacleTasks();
}

void processObstacleTasks(){
  // Updated threshold from <2 to <OBSTACLE_THRESHOLD (20cm) for realistic detection
  bool leftBlocked  = (distances[0] < 200);
  bool rearBlocked  = (distances[1] < 200);
  bool rightBlocked = (distances[2] < 200);

   unsigned long current_millis = millis();  

  if (leftBlocked && !rightBlocked && !rearBlocked && current_millis - previous_millis >= intervals) {   
    previous_millis = current_millis;
      left_indicator();
    }  
    
  } else if (!leftBlocked && rightBlocked && !rearBlocked && current_millis - right_indicating_duration >= right_interval) {
    
     
    right_indicator();
  } else if (!leftBlocked && !rightBlocked && rearBlocked) {
    rear_indicator();
  } else if (leftBlocked && rightBlocked && !rearBlocked) {
    dual_indicator();
  } else if (!leftBlocked && rightBlocked && rearBlocked) {
    Rg_re_indicator();
  } else if (leftBlocked && !rightBlocked && rearBlocked) {
    Lf_re_indicator();
  } else if (!leftBlocked && !rightBlocked && !rearBlocked) {  
    digitalWrite(green_indicator, HIGH);
  } else if (leftBlocked && rightBlocked && rearBlocked) {    
    code_red();                                  
  }
}



void dual_indicator(){
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, LOW);
}

void code_red(){
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
}

void left_indicator(){
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(rear_red_indicator, LOW);
}

void right_indicator(){
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, LOW);
}

void rear_indicator(){
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
}

void Lf_re_indicator(){
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
}

void Rg_re_indicator(){
  digitalWrite(green_indicator, LOW);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(right_red_indicator, blinkState ? HIGH : LOW);
  digitalWrite(rear_red_indicator, blinkState ? HIGH : LOW);
}
