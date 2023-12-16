#ifndef TRAFAG_HH
#define TRAFAG_HH

// #include <FreeRTOS.h>
#include <Arduino.h>
#include "pinout.h"

/* Class for Trafag 8252 pressure sensor*/
class Trafag8252
{
    private:
    const uint8_t pressurePinADC;

    public:
    Trafag8252(uint8_t _pressurePinADC);
    float getPressure();
};

#endif