#ifndef PINOUT_HH
#define PINOUT_HH

// #include "freertos/FreeRTOS.h"
#include "Arduino.h"
#include <driver/adc.h>
#include "esp_adc_cal.h"

#define ERROR_LED 0
#define STATUS_LED 0

// IGNITER //
#define CONTINUITY ADC1_CHANNEL_6
#define IGNITER 4

// DC motors //
#define DCIN1 25
#define DCIN2 33
#define LIM_SW_1 5  // closed
#define LIM_SW_2 18 // open
#define DC_PWM1 0

// PRESSURE SENS //
#define PRESS_SENS1 ADC1_CHANNEL_0
#define PRESS_SENS2 ADC1_CHANNEL_4

// BUZZER //
#define BUZZER 2

// SD CARD //
#define MOSI 13
#define MISO 12
#define SCK 14
#define SD_CS 15

// LOAD CELL //
#define LC1_DT 16
#define LC1_CLK 17

// THERMOCOUPLES
#define THERMO1_CS 27
#define THERMO2_CS 26

// BATTERY
#define BATT_CHECK ADC1_CHANNEL_3

// VALVE STATES
#define VALVE_OPEN 2
#define VALVE_BETWEEN 1
#define VALVE_CLOSE 0

// DEFAULT VREF FOR ADC1
#define DEFAULT_VREF 1100

void beepBoop(int delay, int times);
void pinInit();
enum loadCells
{
    _mainLoadCell = 0,
    _oxidizerLoadCell
};
#endif