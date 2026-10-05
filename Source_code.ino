#include <NewPing.h>

#define SONAR_NUM 3   //number of sensors
#define Max_Dist 200
#define PING_INTERVAL 33

NewPing sonar[SONAR_NUM] = {
  NewPing(2, 3, Max_Dist), // Sensor 1: Left
  NewPing(4, 5, Max_Dist), // Sensor 2: Center
  NewPing(5, 6, Max_Dist)  // Sensor 3: right
};


int green_indicator = 7;
int left_red_indicator = 8;
int right_red_indicator = 9;
int rear_red_indicator = 10;

int distances[SONAR_NUM] = {0, 0, 0};

  unsigned long pingTimer = 0;
  uint8_t currentSensor = 0;

unsigned long previous_millis = 0;
const long right_interval = 6000;

unsigned long left_millis = 0;
const long left_interval = 6000;

unsigned long rear_millis = 0;
const long rear_interval = 6000;

unsigned long Rg_re_millis = 0;
const long Rg_re_interval = 6000;

unsigned long Lf_re_millis = 0;
const long Lf_re_interval = 6000;

unsigned long dual_indicating_millis = 0;
const long dual_indicating_interval = 6000;


void setup() {

  Serial.begin(9600);
  Serial.println("Initializing 3 Ultrasonic Sensors...");

  pingTimer = millis(); //Initializing Start timer

  pinMode(green_indicator, OUTPUT);
  pinMode(left_red_indicator, OUTPUT);
  pinMode(right_red_indicator, OUTPUT;
  pinMode(rear_red_indicator,OUTPUT);


}

void loop() {

  if (millis() - pingTimer >= PING_INTERVAL) {
    pingTimer = millis();

    int reading = sonar[currentSensor].ping_cm();
    distances[currentSensor] = (reading == 0) ? Max_Dist : reading;

    currentSensor++;
    if (currentSensor >= SONAR_NUM) {
      currentSensor = 0;
    }
  }
  
  processObstacleTasks();
}

void processObstacleTasks(){
  bool leftBlocked = (distances[0] < 2);
  bool rearBlocked = (distances[1] < 2);
  bool rightBlocked = (distances[2] < 2);

  if (leftBlocked && !rightBlocked && !rearBlocked) {   
    left_indicator();
  }else if (!leftBlocked && rightBlocked && !rearBlocked) {
    right_indicator();
  }else if (!leftBlocked && !rightBlocked && rearBlocked){
    rear_indicator();
  }else if (leftBlocked && rightBlocked && !rearBlocked) {
    dual_indicator();
  }else if (!leftBlocked && rightBlocked && rearBlocked) {
    Rg_re_indicator();
  }else if (leftBlocked && !rightBlocked && rearBlocked) {
    Lf_re_indicator();
  }else if (!leftBlocked && !rightBlocked && !rearBlocked) {  
    digitalWrite(green_indicator, HIGH);                      
  }else if (leftBlocked && rightBlocked && rearBlocked) {    
    code_red();                                  
  }
}

void dual_indicator(){

  digitalWrite(left_red_indicator, LOW);   //Blinking of Left LED on dashboard
  digitalWrite(left_red_indicator, HIGH);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(left_red_indicator, HIGH);

  digitalWrite(right_red_indicator, LOW);   //Blinking of right LED on dashboard
  digitalWrite(right_red_indicator, HIGH);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(right_red_indicator, HIGH);

}

void code_red(){

  digitalWrite(left_red_indicator, LOW);   //Blinking of left LED on dashboard
  digitalWrite(left_red_indicator, HIGH);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(left_red_indicator, HIGH);

  digitalWrite(right_red_indicator, LOW);   //Blinking of right LED on dashboard
  digitalWrite(right_red_indicator, HIGH);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(right_red_indicator, HIGH);

  digitalWrite(rear_red_indicator, LOW);    //Blinking of rear LED on dashboard
  digitalWrite(rear_red_indicator, HIGH);
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(rear_red_indicator, HIGH);

}



void left_indicator(){
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(left_red_indicator, HIGH);
  digitalWrite(left_red_indicator, LOW);
  digitalWrite(left_red_indicator, HIGH);
  digitalWrite(left_red_indicator, LOW);
}


void right_indicator(){
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(right_red_indicator, HIGH);
  digitalWrite(right_red_indicator, LOW);
  digitalWrite(right_red_indicator, HIGH);
  digitalWrite(right_red_indicator, LOW);
}

void rear_indicator(){
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(rear_red_indicator, HIGH);
  digitalWrite(rear_red_indicator, LOW);
  digitalWrite(rear_red_indicator, HIGH);
  digitalWrite(rear_red_indicator, LOW);
}

void Lf_re_indicator(){
  left_indicator();
  rear_indicator();
}

void Rg_re_indicator(){
  right_indicator();
  rear_indicator();
}
