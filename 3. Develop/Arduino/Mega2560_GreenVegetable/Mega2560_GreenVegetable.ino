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

const int LED7_DATA = 6;
const int LED7_LATCH = 2;
const int LED7_CLK = 5;

// initialize Timer
const int ACT_HOUR = 22;
const int C_SECOND = 1;
const int C_MINUTE = C_SECOND * 60;
const int C_HOUR = C_MINUTE * 60;
// Test const int C_HOUR = C_SECOND * 5;

// config time
const int PUMP_TIMER_MAX = 6 * C_HOUR;
const int PUMP_RUN_TIME = 1 * C_HOUR;
const int LIGHT_TIMER_MAX = 3 * C_HOUR;
const int LIGHT_RUN_TIME = 1 * C_HOUR;

int old_hour, old_sec;
bool pump_active, light_active;
unsigned int pump_active_counter;
unsigned int light_active_counter;

volatile unsigned int hour_timer, min_timer, sec_timer;
volatile unsigned int pump_counter = 0;
volatile unsigned int light_counter = 0;

// 7 LED initialize
unsigned char LED_CODE[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};

// the setup routine runs once when you press reset:
void setup() {
  // initialize Timer
  Timer1.initialize(991775);    // 1s
  Timer1.attachInterrupt(blinkLed);
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // initialize digital pin LED_BUILDIN as an output
  sec_timer = 55;
  min_timer = 59;
  hour_timer = 21;

  old_sec = sec_timer;
  pump_active = false;
  light_active = false;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DIG_22, OUTPUT);
  pinMode(DIG_24, OUTPUT);
  pinMode(DIG_26, OUTPUT);
  pinMode(DIG_28, OUTPUT);

  pinMode(LED7_DATA, OUTPUT);
  pinMode(LED7_LATCH, OUTPUT);
  pinMode(LED7_CLK, OUTPUT);

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
}

// the loop routine runs over and over again forever:
void loop() {
  unsigned int hour_timer_copy;
  unsigned int min_timer_copy;
  unsigned int sec_timer_copy;

  noInterrupts();
  hour_timer_copy = hour_timer;
  min_timer_copy = min_timer;
  sec_timer_copy = sec_timer;
  interrupts();

  // Blink led build-in, turn the LED on (HIGH is the voltage level)
  if ((sec_timer_copy % 2) == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Run pump
  if ((hour_timer_copy == 4 || hour_timer_copy == 10 || hour_timer_copy == 16 || hour_timer_copy == 22)
        && min_timer_copy == 0 && sec_timer_copy == 0){
    if (pump_active == false) {
      digitalWrite(DIG_22, HIGH);
      digitalWrite(DIG_24, HIGH);
      pump_active = true;
      pump_active_counter = 0;
      Serial.println("********** Turn ON pump **********");
    }
  }
  if (pump_active == true) {
    if (old_sec != sec_timer_copy) {
      if (pump_active_counter == PUMP_RUN_TIME) {
        digitalWrite(DIG_22, LOW);
        digitalWrite(DIG_24, LOW);
        pump_active = false;
        Serial.println("********** Turn OFF pump **********");
      } else {
        pump_active_counter++;
      }
    }
  }

  // Run light
  if ((hour_timer_copy == 1 || hour_timer_copy == 4 || hour_timer_copy == 19 || hour_timer_copy == 22)
        && min_timer_copy == 0 && sec_timer_copy == 0){
    if (light_active == false) {
      digitalWrite(DIG_26, HIGH);
      digitalWrite(DIG_28, HIGH);
      light_active = true;
      light_active_counter = 0;
      Serial.println("********** Turn ON light **********");
    }
  }
  if (light_active == true) {
    if (old_sec != sec_timer_copy) {
      if (light_active_counter == LIGHT_RUN_TIME) {
        digitalWrite(DIG_26, LOW);
        digitalWrite(DIG_28, LOW);
        light_active = false;
        Serial.println("********** Turn OFF light **********");
      } else {
        light_active_counter++;
      }
    }
  }
    
//  // read the input on analog pin 0:
//  int sensorValue = analogRead(A1);
//  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
//  float voltage = sensorValue * (5.0 / 1023.0);
//
//  // print out the value you read:
//  Serial.println(sensorValue);
  
  old_sec = sec_timer_copy;

  Led7Display(min_timer_copy, sec_timer_copy, true);
    
  //delay(100);        // delay in between reads for stability
}

void ShowDigital(int num, int value, int dot){
  unsigned char temp;
  temp = LED_CODE[value];
  if (dot == 1) temp &= 0x7f;
  switch(num) {
    case 0: {
      digitalWrite(LED7_LATCH, LOW);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xfd);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, temp);
      digitalWrite(LED7_LATCH, HIGH);
    } break;
    case 1: {
      digitalWrite(LED7_LATCH, LOW);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xfb);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, temp);
      digitalWrite(LED7_LATCH, HIGH);
    } break;
    case 2: {
      digitalWrite(LED7_LATCH, LOW);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xef);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, temp);
      digitalWrite(LED7_LATCH, HIGH);
    } break;
    case 3: {
      digitalWrite(LED7_LATCH, LOW);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xdf);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, temp);
      digitalWrite(LED7_LATCH, HIGH);
    } break;
    case 4: { // turn on ":"
      digitalWrite(LED7_LATCH, LOW);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xf7);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0x00);
      digitalWrite(LED7_LATCH, HIGH);
    } break;    
    case 5: { // turn off ":"
      digitalWrite(LED7_LATCH, LOW);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xfe);
      shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0x00);
      digitalWrite(LED7_LATCH, HIGH);
    } break;    
  }
}

void Led7Display(int num1, int num2, bool clk){
  int i;
  int l1, l2, l3, l4;
  if (clk == true){
    l1 = num1 / 10;
    l2 = num1 % 10;
    l3 = num2 / 10;
    l4 = num2 % 10;
    for (i = 0; i < 25; i++){
      ShowDigital(0, l1, 0); delay(5);
      ShowDigital(1, l2, 0); delay(5);
      ShowDigital(2, l3, 0); delay(5);
      ShowDigital(3, l4, 0); delay(5);
      if ((l4%2) == 0) {
        ShowDigital(4, 0, 0); delay(5);
      } else {
        ShowDigital(5, 0, 0); delay(5);
      }
    }
  } else {
    
  }
}


