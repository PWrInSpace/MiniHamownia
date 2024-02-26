#ifndef CONTINITYCHECK_HH
#define CONTINITYCHECK_HH

#include <driver/adc.h>
#include "esp_adc_cal.h"
#include "pinout.h"

/* Class for Igniter Continuity sensor*/
class ContinuityCheck
{
    private:
    const adc1_channel_t continuityPinADC;
    esp_adc_cal_characteristics_t *adc_chars;

    public:
    ContinuityCheck(adc1_channel_t _continuityPinADC);
    bool getContinuity();
};

#endif