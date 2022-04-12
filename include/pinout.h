#ifndef PINOUT_HH
#define PINOUT_HH

#include "FreeRTOS.h"
#include "Arduino.h"

#define ERROR_LED 0
#define STATUS_LED 0

// IGNITER //
#define CONTINUITY 34
#define IGNITER 4

// DC motors //
#define DCIN1 25
#define DCIN2 33
#define LIM_SW_1 5 //closed
#define LIM_SW_2 18 //open
#define DC_PWM1 0

// PRESSURE SENS //
#define PRESS_SENS 36

// BUZZER //
#define BUZZER 2

// SD CARD //
#define MOSI 13
#define MISO 12
#define SCK 14
#define SD_CS 15

// LOAD CELLS //
#define LC1_DT 16
#define LC1_CLK 17

#define LC2_DT 35        
#define LC2_CLK 32

// THERMOCOUPLES
#define THERMO1_CS 27   
#define THERMO2_CS 26

// BATTERY
#define BATT_CHECK 39   

// SERVO
#define SERVO_OPEN_POSITION 20
#define SERVO_CLOSE_POSITION 105
#define SERVO_PIN 27

// VALVE STATES
#define VALVE_OPEN 2
#define VALVE_BETWEEN 1
#define VALVE_CLOSE 0

void beepBoop(int delay, int times);
float checkBattery(uint8_t batteryPin, const float &reverseDividerVal);
void pinInit();

#endif