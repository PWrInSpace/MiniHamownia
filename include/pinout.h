#ifndef PINOUT_HH
#define PINOUT_HH

#define DOUT 15 //pin 3 Arduino i wyjście DAT czujnika
#define CLK 26  //pin 2 Arduino i wyjście CLK czujnika
#define ERROR_LED 2
#define STATUS_LED 2

// igniter
#define CONTINUITY 34
#define IGNITER 19

// DC motors
#define DCIN1 22
#define DCIN2 21
#define LIM_SW_1 12
#define LIM_SW_2 13
#define DC_PWM1 0

// PRESSURE SENS
#define PRESS_SENS 36

//Buzzer
#define BUZZER 2

// mySD
#define MOSI 27
#define MISO 25
#define SCK 26
#define SD_CS 33

// VALVE STATES
#define VALVE_OPEN 2
#define VALVE_BETWEEN 1
#define VALVE_CLOSE 0

#endif