#ifndef PINOUT_HH
#define PINOUT_HH

#include "FreeRTOS.h"
#include "Arduino.h"

#define DOUT 15 //pin 3 Arduino i wyjście DAT czujnika
#define CLK 26  //pin 2 Arduino i wyjście CLK czujnika
#define ERROR_LED 2
#define STATUS_LED 2

// igniter
#define CONTINUITY 34
#define IGNITER 19

// DC motors
#define DCIN1 21
#define DCIN2 22
#define LIM_SW_1 13
#define LIM_SW_2 12
#define DC_PWM1 0

// PRESSURE SENS
#define PRESS_SENS 36

//Buzzer
#define BUZZER 14

// mySD
#define MOSI 27
#define MISO 25
#define SCK 26
#define SD_CS 33

// VALVE STATES
#define VALVE_OPEN 2
#define VALVE_BETWEEN 1
#define VALVE_CLOSE 0

// LOAD CELLS
#define LC1_DT 15
#define LC1_CLK 5

#define LC2_DT 0        // for future
#define LC2_CLK 0

// THERMOCOUPLES
#define THERMO1_CS 0    // for future
#define THERMO2_CS 0

// BATTERY
#define BATT_CHECK 0    // for future

void beepBoop(int delay, int times);
float checkBattery(uint8_t batteryPin, const float &reverseDividerVal);
void pinInit();

#endif