#ifndef LOOP_TASKS_HH
#define LOOP_TASKS_HH

// System includes
#include <Arduino.h>
#include <FreeRTOS.h>
#include <BluetoothSerial.h>

// Main includes
#include "../pinout.h"
#include "../btUI.h"
#include "../stateMachine.h"

// Hardware includes
#include "../hardware/SDcard.h"
#include "../hardware/max6675.h"
#include "../hardware/trafag8252.h"
#include "../hardware/DCValve.h"

// External includes
#include "HX711_ADC.h"

extern BluetoothUI btUI;
extern StateMachine sm;
extern DCValve firstValve;
extern DCValve secondValve;
extern SPIClass myspi;

void btReceiveTask(void *arg);
void btTransmitTask(void *arg);
void uiTask(void *arg);

void stateTask(void *arg);
void dataTask(void *arg);
void sdTask(void *arg);
void staticFireTask(void *arg);
void calibrationTask(void *arg);
void timeServoOpen(void *arg);

#endif