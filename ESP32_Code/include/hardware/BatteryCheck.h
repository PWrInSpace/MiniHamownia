#ifndef BATTERYCHECK_HH
#define BATTERYCHECK_HH

#include <driver/adc.h>
#include "esp_adc_cal.h"
#include "pinout.h"

/* Class for Igniter Continuity sensor*/
class BatteryCheck
{
    private:
    const adc1_channel_t batteryPinADC;
    const uint16_t voltageDivider;
    esp_adc_cal_characteristics_t *adc_chars;

    public:
    BatteryCheck(adc1_channel_t _batteryPinADC, uint16_t _voltageDivider);
    bool getBattery();
};

#endif