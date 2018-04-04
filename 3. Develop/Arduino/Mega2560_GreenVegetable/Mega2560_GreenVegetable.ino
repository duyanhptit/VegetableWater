/* Project Green Vegetable
    Author: AnhDD
    Date: 16/03/2018
    Address: Hi-tech Park Hoa Lac, Ha Noi
*/

#include "TimerOne.h"


// initialize MEGA_2560 pins
const int DIG_22 = 22;
const int DIG_24 = 24;
const int DIG_26 = 26;
const int DIG_28 = 28;

// initialize Timer
const int ACT_HOUR = 22;
const int C_SECOND = 1;
const int C_MINUTE = C_SECOND * 60;
const int C_HOUR = C_MINUTE * 60;
// Test const int C_HOUR = C_SECOND * 5 ;

// config time
const int PUMP_TIMER_MAX = 6 * C_HOUR;
const int PUMP_RUN_TIME = 1 * C_HOUR;
const int LIGHT_TIMER_MAX = 3 * C_HOUR;
const int LIGHT_RUN_TIME = 1 * C_HOUR;

int state;
int pump_active_counter;
int light_active_counter;
volatile unsigned int hour_timer, min_timer, sec_timer;
volatile unsigned int pump_counter = 0;
volatile unsigned int light_counter = 0;

// the setup routine runs once when you press reset:
void setup() {
  // initialize Timer
  Timer1.initialize(991775);    // 1s
  Timer1.attachInterrupt(blinkLed);
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // initialize digital pin LED_BUILDIN as an output
  state = HIGH;
  sec_timer = 0;
  min_timer = 47;
  hour_timer = 12;

  pump_active_counter = 0;
  light_active_counter = 0;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DIG_22, OUTPUT);
  pinMode(DIG_24, OUTPUT);
  pinMode(DIG_26, OUTPUT);
  pinMode(DIG_28, OUTPUT);

  Serial.println("Hello Duy Anh!");
  Serial.println("Config Done - Begin the loop.");

  delay(3000);

  digitalWrite(DIG_22, HIGH);
  digitalWrite(DIG_24, HIGH);
  digitalWrite(DIG_26, HIGH);
  digitalWrite(DIG_28, HIGH);
}


void blinkLed(void) {
  // O'clock
  sec_timer = (sec_timer + 1) % 60;
  if (sec_timer == 0) {
    min_timer = (min_timer + 1) % 60;
    if (min_timer == 0) {
      hour_timer = (hour_timer + 1) % 24;
    }
  }
  Serial.print("It's ");
  Serial.print(hour_timer);
  Serial.print(":");
  Serial.print(min_timer);
  Serial.print(":");
  Serial.println(sec_timer);

  
  // counter pump
  if (pump_counter == PUMP_TIMER_MAX - 1) {
    pump_counter = 0;
  } else {
    pump_counter = pump_counter + 1;
  }
  // counter light
  if (light_counter == LIGHT_TIMER_MAX - 1) {
    light_counter = 0;
  } else {
    light_counter = light_counter + 1;
  }

  // turn the LED on (HIGH is the voltage level)
  if (state == HIGH) {
    state = LOW;
  } else if (state == LOW) {
    state = HIGH;
  }
  digitalWrite(LED_BUILTIN, state);
}

// the loop routine runs over and over again forever:
void loop() {
  unsigned int hour_timer_copy;
  unsigned int pump_counter_copy;
  unsigned int light_counter_copy;

  noInterrupts();
  hour_timer_copy = hour_timer;
  pump_counter_copy = pump_counter;
  light_counter_copy = light_counter;
  interrupts();

  // Run pump
  if (pump_counter_copy == 0) {
    digitalWrite(DIG_22, HIGH);
    digitalWrite(DIG_24, HIGH);
  } else if (pump_counter_copy == PUMP_RUN_TIME) {
    digitalWrite(DIG_22, LOW);
    digitalWrite(DIG_24, LOW);
  }

  // Run light
  if (light_counter_copy == 0) {
    if (hour_timer_copy >= 18 || hour_timer_copy <= 6) {
      digitalWrite(DIG_26, HIGH);
      digitalWrite(DIG_28, HIGH);
    }
  } else if (light_counter_copy == LIGHT_RUN_TIME) {
    digitalWrite(DIG_26, LOW);
    digitalWrite(DIG_28, LOW);
  }
    
//  // read the input on analog pin 0:
//  int sensorValue = analogRead(A1);
//  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
//  float voltage = sensorValue * (5.0 / 1023.0);
//
//  // print out the value you read:
//  Serial.println(sensorValue);
  delay(100);        // delay in between reads for stability
}
