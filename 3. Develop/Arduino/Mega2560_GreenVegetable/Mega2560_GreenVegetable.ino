/* Project Green Vegetable
    Author: AnhDD
    Date: 16/03/2018
    Address: Hi-tech Park Hoa Lac, Ha Noi
*/

#include "TimerOne.h"
#include "Wire.h"

// TDS Sensor Define
#define TDSSensorPin1 A14
#define TDSSensorPin2 A15
#define Vref 5.0
#define TDS_SAM_SIZE 30

// initialize MEGA_2560 pins
#define LIGHT_1 44        // Relay_220 1
#define LIGHT_2 46        // Relay_220 2
#define LIGHT_3 48        // Relay_220 3
#define LIGHT_4 49        // Relay_220 4
#define PUMP 47           // Relay_220 5
#define RELAY220 45       // Relay_220 6

#define VALVE_WATER 39    // Relay_26V 1
#define VALVE_1 38        // Relay_26V 2
#define VALVE_2 41        // Relay_26V 3
#define VALVE_3 40        // Relay_26V 4
#define VALVE_BLEND 43    // Relay_26V 5
#define VALVE 42          // Relay_26V 6

#define LED7_DATA 4
#define LED7_LATCH 3
#define LED7_CLK 2

#define MENU_BT 27
#define UP_BT 28
#define DOWN_BT 29

int TDSRaw_buffer[TDS_SAM_SIZE];
int TDSTemp_buffer[TDS_SAM_SIZE];
int TDS_buffer_index = 0, copy_index = 0;
float averageVoltage = 0, TDSValue = 0, temperature = 25;

// initialize Menu Control
const int NUM_OF_MENU = 4; // 0 - Clock; 1 - TDS; 2 - Light time setup; 3 - Pump time setup
int menu_index = 0, pressed_menu_cnt = 0;
bool input_menu = false, pressed_menu = false, holded_menu = false;
bool input_up = false, pressed_up = false;
bool input_down = false, pressed_down = false;
bool set_hour = false, set_min = false;
int fix_hour, fix_min;
bool set_TDS = false, set_light = false, set_pump = false;

// config time
int CYCLE_LIGHT = 4;
int CYCLE_PUMP = 3;
int TDS_SETUP = 1100;

// initialize RTC DS-1307
const byte DS1307 = 0x68;
const byte NumberOfFields = 7;
int second, minute, hour, day, wday, month, year;

unsigned long blinkTimePoint;
bool blk;

// 7 LED initialize
unsigned char LED_CODE[13] = {0x3f, // 0
                              0x06, // 1
                              0x5b, // 2
                              0x4f, // 3
                              0x66, // 4
                              0x6d, // 5
                              0x7d, // 6
                              0x07, // 7
                              0x7f, // 8
                              0x6f, // 9
                              0x00, // none
                              0x5e, // d
                              0x7c  // b
};
#define C_H_SIZE 7
int CYCLE_HOUR[C_H_SIZE] = { 2, // period 2h
                             3, // period 3h
                             4, // period 4h
                             6, // period 6h
                             8, // period 8h
                             12,// period 12h
                             24 // period 14h
};
#define H_V_SIZE 12
int CYCLE_HOUR_VALUE[C_H_SIZE][H_V_SIZE] = { { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23},        // 2h
                                             { 0, 3, 6, 9, 12, 15, 18, 21, 24, 24, 24, 24},       // 3h
                                             { 2, 6, 10, 14, 18, 22, 24, 24, 24, 24, 24, 24},     // 4h
                                             { 2, 8, 14, 20, 24, 24, 24, 24, 24, 24, 24, 24},     // 6h
                                             { 2, 10, 18, 24, 24, 24, 24, 24, 24, 24, 24, 24},    // 8h
                                             { 6, 18, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24},    // 12h
                                             { 6, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24}     // 24h
};

//==================================================
// the setup routine runs once when you press reset:
//==================================================
void setup() {
  initRTC();

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(LIGHT_1, OUTPUT);
  pinMode(LIGHT_2, OUTPUT);
  pinMode(LIGHT_3, OUTPUT);
  pinMode(LIGHT_4, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(RELAY220, OUTPUT);

  pinMode(VALVE_WATER, OUTPUT);
  pinMode(VALVE_1, OUTPUT);
  pinMode(VALVE_2, OUTPUT);
  pinMode(VALVE_3, OUTPUT);
  pinMode(VALVE_BLEND, OUTPUT);
  pinMode(VALVE, OUTPUT);

  pinMode(LED7_DATA, OUTPUT);
  pinMode(LED7_LATCH, OUTPUT);
  pinMode(LED7_CLK, OUTPUT);

  pinMode(MENU_BT, INPUT);
  pinMode(UP_BT, INPUT);
  pinMode(DOWN_BT, INPUT);

  pinMode(TDSSensorPin1, INPUT);
  pinMode(TDSSensorPin2, INPUT);

  blinkTimePoint = millis();
  blk = false;

  Serial.println("Hello Duy Anh!");
  Serial.println("Config Done - Begin the loop.");

  delay(1000);
}

void initRTC() {
  Wire.begin();
  //setTime(18, 28, 0, 3, 7, 8, 18);   // 16:12:00 T3 07-08-2018
}

// Set time for Module DS1307
void setTime(byte hr, byte minu, byte sec, byte wd, byte d, byte mth, byte yr)
{
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0x00));       // re-setup pointer
  Wire.write(dec2bcd(sec));
  Wire.write(dec2bcd(minu));
  Wire.write(dec2bcd(hr));
  Wire.write(dec2bcd(wd));      // day of week: Sunday = 1, Saturday = 7
  Wire.write(dec2bcd(d));
  Wire.write(dec2bcd(mth));
  Wire.write(dec2bcd(yr));
  Wire.endTransmission();
}

//===================================================
// the loop routine runs over and over again forever:
//===================================================
void loop() {
  //=== Blink led build-in, turn the LED on (HIGH is the voltage level)
  if ((second % 2) == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
  
  //=== Read time from Module DS1307
  readRD1307();
  //digitalClockDisplay();
  caculateTDSValue();

  MenuControl();
  ControlLight();
  ControlPump();

}

void MenuControl() {
    if (digitalRead(MENU_BT) == HIGH) { // press menu
    input_menu = true;
    pressed_menu_cnt++;
    if (pressed_menu_cnt > 10) { // hold menu active
      holded_menu = true;
    }
  } else {    // unpress menu
    if (holded_menu == true) {        // hold menu active state
      pressed_menu = false;
      holded_menu = false;
    } else if (input_menu == true) {  // press menu active state
      pressed_menu = true;
    } else {                          // idle menu
      holded_menu = false;
      pressed_menu = false;
    }
    input_menu = false;
    pressed_menu_cnt = 0;
  }
  
  if (digitalRead(UP_BT) == HIGH) {
    input_up = true;
  } else {
    if (input_up == true) {
      pressed_up = true;
    } else {
      pressed_up = false;
    }
    input_up = false;
  }
  if (digitalRead(DOWN_BT) == HIGH) {
    input_down = true;
  } else {
    if (input_down == true) {
      pressed_down = true;
    } else {
      pressed_down = false;
    }
    input_down = false;
  }
  
  switch (menu_index) {
    case 0: { // Display Display and setting clock
      if (holded_menu == true) {
        set_hour = true;
        fix_hour = hour;
        fix_min = minute;
      }
      
      if (set_hour == true) { // Set hour state
        if (pressed_up == true) {
          fix_hour++;
          fix_hour = (fix_hour + 24) % 24;
        }
        if (pressed_down == true) {
          fix_hour--;
          fix_hour = (fix_hour + 24) % 24;
        }
        Led7Display(fix_hour, fix_min, 0, 12);
        if (pressed_menu == true) {
          set_min = true;
          set_hour = false;
        }
      } else if (set_min == true) { // Set minute state
        // Todo set minute
        if (pressed_up == true) {
          fix_min++;
          fix_min = (fix_min + 60) % 60;
        }
        if (pressed_down == true) {
          fix_min--;
          fix_min = (fix_min + 60) % 60;
        }
        Led7Display(fix_hour, fix_min, 0, 34);
        if (pressed_menu == true) {
          set_min = false;
          set_hour = false;
          // Setup time
          setTime(fix_hour, fix_min, 0, wday, day, month, year);
        }
      } else { // Show clock state
        Led7Display(hour, minute, 0, 5);
        if (pressed_menu == true) {
          menu_index++;
        }
      }
    } break;
    case 1: { // Display and setting TDS Value
      if (holded_menu == true) {
        set_TDS = true;
      }
      
      if (set_TDS == true) {  // Set TDS Lever
        if (pressed_up == true) {
          TDS_SETUP += 10;
          TDS_SETUP = (TDS_SETUP + 10000) % 10000;
        }
        if (pressed_down == true) {
          TDS_SETUP -= 10;
          TDS_SETUP = (TDS_SETUP + 10000) % 10000;
        }
        Led7Display(TDS_SETUP, 0, 1, 0);
        if (pressed_menu == true) {
          set_TDS = false;
        }
      } else {
        Led7Display(TDSValue, 0, 1, 0);
        if (pressed_menu == true) {
          menu_index++;
        }
      }
    } break;
    case 2: { // Display and setting Light Time Cycle
      if (holded_menu == true) {
        set_light = true;
      }
      
      if (set_light == true) {  // Set Light Time Cycle
        if (pressed_up == true) {
          CYCLE_LIGHT++;
          CYCLE_LIGHT = (CYCLE_LIGHT + C_H_SIZE) % C_H_SIZE;
        }
        if (pressed_down == true) {
          CYCLE_LIGHT--;
          CYCLE_LIGHT = (CYCLE_LIGHT + C_H_SIZE) % C_H_SIZE;
        }
        Led7Display(CYCLE_HOUR[CYCLE_LIGHT], 0, 3, 34);
        if (pressed_menu == true) {
          set_light = false;
        }
      } else {
        Led7Display(CYCLE_HOUR[CYCLE_LIGHT], 0, 3, 0);
        if (pressed_menu == true) {
          menu_index++;
        }
      }
    } break;
    case 3: { // Display and setting Pump Time Cycle
      if (holded_menu == true) {
        set_pump = true;
      }
      
      if (set_pump == true) {  // Set Pump Time Cycle
        if (pressed_up == true) {
          CYCLE_PUMP++;
          CYCLE_PUMP = (CYCLE_PUMP + C_H_SIZE) % C_H_SIZE;
        }
        if (pressed_down == true) {
          CYCLE_PUMP--;
          CYCLE_PUMP = (CYCLE_PUMP + C_H_SIZE) % C_H_SIZE;
        }
        Led7Display(CYCLE_HOUR[CYCLE_PUMP], 0, 4, 34);
        if (pressed_menu == true) {
          set_pump = false;
        }
      } else {
        Led7Display(CYCLE_HOUR[CYCLE_PUMP], 0, 4, 0);
        if (pressed_menu == true) {
          menu_index = 0;
        }
      }
    } break;
  }
}

void ShowDigital(int num, int value, bool dot) {
  unsigned char temp;
  temp = LED_CODE[value];
  if (dot == true) temp |= 0x80;

  switch (num) {
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
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xf7);
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, temp);
        digitalWrite(LED7_LATCH, HIGH);
    } break;
    case 3: {
        digitalWrite(LED7_LATCH, LOW);
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xef);
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, temp);
        digitalWrite(LED7_LATCH, HIGH);
    } break;
    case 4: { // turn on ":"
        digitalWrite(LED7_LATCH, LOW);
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0x7f);
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0x00);
        digitalWrite(LED7_LATCH, HIGH);
    } break;
    case 5: { // turn off ":"
        digitalWrite(LED7_LATCH, LOW);
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xff);
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0x00);
        digitalWrite(LED7_LATCH, HIGH);
    } break;
    others: {
        digitalWrite(LED7_LATCH, LOW);
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0xff);
        shiftOut(LED7_DATA, LED7_CLK, MSBFIRST, 0x00);
        digitalWrite(LED7_LATCH, HIGH);      
    } break;
  }
}

// Var option: 0 - clock, 1 - num 1, 2 - num 2, 3 - light, 4 - pump
// Var blink: 0 - none, 1 - L7_1, 2 - L7_2, 3 - L7_3, 4 - L7_4, 5 - Colon,
void Led7Display(int num1, int num2, int option, int blk_opt) {
  int i;
  int l1, l2, l3, l4;
  unsigned long timePoint;

  switch (option) {
    case 0: {
      l1 = num1 / 10;
      l1 = (l1 == 0) ? 10 : l1;
      l2 = num1 % 10;
      l3 = num2 / 10;
      l4 = num2 % 10;
    } break;
    case 1: {
      l1 = num1 / 1000;
      l1 = (l1 == 0) ? 10: l1;
      l2 = num1 / 100 % 10;
      l2 = (l1 == 10 && l2 == 0) ? 10: l2;
      l3 = num1 / 10 % 10;
      l3 = (l1 == 10 && l2 == 10 && l3 == 0) ? 10: l3;
      l4 = num1 % 10;
    } break;
    case 2: {
      l1 = num2 / 1000;
      l1 = (l1 == 0) ? 10: l1;
      l2 = num2 / 100 % 10;
      l2 = (l1 == 10 && l2 == 0) ? 10: l2;
      l3 = num2 / 10 % 10;
      l3 = (l1 == 10 && l2 == 10 && l3 == 0) ? 10: l3;
      l4 = num2 % 10;
    } break;
    case 3: {
      l1 = 11; // d
      l2 = 10;
      l3 = num1 / 10;
      l3 = (l3 == 0) ? 10: l3;
      l4 = num1 % 10;
    } break;
    case 4: {
      l1 = 12; // b
      l2 = 10;
      l3 = num1 / 10;
      l3 = (l3 == 0) ? 10: l3;
      l4 = num1 % 10;
    } break;
  }
  
  timePoint = millis();
  if (timePoint - blinkTimePoint > 300U) {
    blinkTimePoint = timePoint;
    blk = !blk;
  }
  
  switch (blk_opt) {
    case 1: {
      l1 = blk ? l1 : 10;
    } break;
    case 2: {
      l2 = blk ? l2 : 10;
    } break;
    case 3: {
      l3 = blk ? l3 : 10;
    } break;
    case 4: {
      l4 = blk ? l4 : 10;
    } break;
    case 12: {
      l1 = blk ? l1 : 10;
      l2 = blk ? l2 : 10;
    } break;
    case 34: {
      l3 = blk ? l3 : 10;
      l4 = blk ? l4 : 10;
    } break;
    others: {

    } break;
  }
  for (i = 0; i < 10; i++) {
    ShowDigital(0, l1, false); delay(3);
    ShowDigital(1, l2, false); delay(3);
    ShowDigital(2, l3, false); delay(3);
    ShowDigital(3, l4, false); delay(3);
    if (option == 0 && blk_opt == 5 && (second % 2) == 0) {
      ShowDigital(4, 0, 0); delay(3);
    } else if (option == 0 && blk_opt != 5) {
      ShowDigital(4, 0, 0); delay(3);
    } else if (option != 0 && option != 1) {
      ShowDigital(4, 0, 0); delay(3);
    } else {
      ShowDigital(5, 0, 0); delay(3);
    }  
  }
}

// Read data from Module DS1307 via I2C Communication
void readRD1307() {
  Wire.beginTransmission(DS1307);
  Wire.write((byte)0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS1307, NumberOfFields);

  second  = bcd2dec(Wire.read() & 0x7f);
  minute  = bcd2dec(Wire.read());
  hour    = bcd2dec(Wire.read() & 0x3f); // mode 24h
  wday    = bcd2dec(Wire.read());
  day     = bcd2dec(Wire.read());
  month   = bcd2dec(Wire.read());
  year    = bcd2dec(Wire.read());
  year  += 2000;
}

// Convert format form BCD (Binary-Coded Decimal) to Decimal
int bcd2dec(byte num) {
  return ((num / 16 * 10) + (num % 16));
}

// Convert format from Decimal BCD to (Binary-Coded Decimal)
int dec2bcd(byte num) {
  return ((num / 10 * 16) + (num % 10));
}

//// Display Digital Clock
//void digitalClockDisplay() {
//  Serial.print(hour);
//  printDigits(minute);
//  printDigits(second);
//  Serial.print(" ");
//  Serial.print(day);
//  Serial.print("/");
//  Serial.print(month);
//  Serial.print("/");
//  Serial.print(year);
//  Serial.println();
//}
//
//// print format o'clock
//void printDigits(int digits) {
//  Serial.print(":");
//  if (digits < 10) Serial.print("0");
//  Serial.print(digits);
//}

void caculateTDSValue() {
  static unsigned long analogSampleTimepoint = millis();
  if(millis() - analogSampleTimepoint > 50U) //every 50 milliseconds, read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    TDSRaw_buffer[TDS_buffer_index] = analogRead(TDSSensorPin2); //read the analog value and store into the buffer
    TDS_buffer_index++;
    if(TDS_buffer_index == TDS_SAM_SIZE)
      TDS_buffer_index = 0;
  }
  
  static unsigned long printTimepoint = millis();
  if(millis() - printTimepoint > 500U)
  {
    printTimepoint = millis();
    for(copy_index = 0; copy_index < TDS_SAM_SIZE; copy_index++)
      TDSTemp_buffer[copy_index]= TDSRaw_buffer[copy_index];
    
    averageVoltage = getMedianNum(TDSTemp_buffer, TDS_SAM_SIZE) * (float)Vref / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient; //temperature compensation
//    TDSValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
    TDSValue = 12.35 * exp(1.948*compensationVolatge); // f(x) = 12.35 * exp(1.948 * x)
    
    Serial.print("voltage:");
    Serial.print(averageVoltage,5);
    Serial.print("V ");
    Serial.print("TDS Value:");
    Serial.print(TDSValue,0);
    Serial.println("ppm");
  }
}

int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

void ControlLight() {
  if (checkTimeActive(CYCLE_LIGHT)) {
    digitalWrite(LIGHT_1, HIGH);
    digitalWrite(LIGHT_2, HIGH);
    digitalWrite(LIGHT_3, HIGH);
    digitalWrite(LIGHT_4, HIGH);
  } else {
    digitalWrite(LIGHT_1, LOW);
    digitalWrite(LIGHT_2, LOW);
    digitalWrite(LIGHT_3, LOW);
    digitalWrite(LIGHT_4, LOW);
  }
}

void ControlPump() {
  if (checkTimeActive(CYCLE_PUMP)) {
    digitalWrite(PUMP, HIGH);
    if ((TDS_SETUP - TDSValue) > 50) {
      digitalWrite(VALVE_BLEND, HIGH);     // LOW is active
      digitalWrite(VALVE_1, HIGH);
      digitalWrite(VALVE_2, HIGH);
    } else {
      digitalWrite(VALVE_BLEND, LOW);    // HIGH is deactive
      digitalWrite(VALVE_1, LOW);
      digitalWrite(VALVE_2, LOW);
    }
  } else {
    digitalWrite(PUMP, LOW);
    digitalWrite(VALVE_BLEND, LOW);      // HIGH is deactive
    digitalWrite(VALVE_1, LOW);
    digitalWrite(VALVE_2, LOW);
  }
}

bool checkTimeActive(int period) {
  int i;
  for (i = 0; i < H_V_SIZE; i++) {
    if (hour == CYCLE_HOUR_VALUE[period][i])
      return true;
  }
  return false;
}




