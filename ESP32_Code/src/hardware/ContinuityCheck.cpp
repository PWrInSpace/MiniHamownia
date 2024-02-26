#include "../include/hardware/ContinuityCheck.h"

ContinuityCheck::ContinuityCheck(adc1_channel_t _continuityPinADC) : 
    continuityPinADC(_continuityPinADC)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(_continuityPinADC, ADC_ATTEN_DB_11);
        adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
        //Characterize ADC at particular atten
    }
 
bool ContinuityCheck::getContinuity()
{
    return ((esp_adc_cal_raw_to_voltage(adc1_get_raw(continuityPinADC), adc_chars) > 1000) ? true : false);
    //return ((analogReadMilliVolts(pressurePinADC) / 1000.0 * voltageDivider) - 0.5) * 17.5; //Output w barach
}   