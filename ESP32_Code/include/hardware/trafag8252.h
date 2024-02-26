#ifndef TRAFAG_HH
#define TRAFAG_HH

// #include <FreeRTOS.h>
#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"
#include "pinout.h"

/* Class for Trafag 8252 pressure sensor*/
class Trafag8252
{
    private:
    const adc1_channel_t pressurePinADC;
    const uint16_t voltageDivider;
    esp_adc_cal_characteristics_t *adc_chars;

    public:
    Trafag8252(adc1_channel_t _pressurePinADC, uint16_t _voltageDivider);
    float getPressure();
};

#endif