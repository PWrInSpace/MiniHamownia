#ifndef TRAFAG_HH
#define TRAFAG_HH

#include <FreeRTOS.h>
#include <Arduino.h>
#include "pinout.h"

/* Class for Trafag 8252 pressure sensor*/
class Trafag8252
{
    private:
    const uint8_t pressurePinADC;
    uint16_t calibrationConstant;

    public:
    Trafag8252(uint8_t _pressurePinADC, uint16_t _calibrationConstant);
    float getPressure();
    float sensorCalibration(const float &pressure);
};

#endif