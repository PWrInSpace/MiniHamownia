#include "../include/hardware/BatteryCheck.h"

BatteryCheck::BatteryCheck(adc1_channel_t _batteryPinADC, uint16_t _voltageDivider) : 
    batteryPinADC(_batteryPinADC), voltageDivider(_voltageDivider)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(_batteryPinADC, ADC_ATTEN_DB_11);
        adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
        //Characterize ADC at particular atten
    }
 
bool BatteryCheck::getBattery()
{
    return (esp_adc_cal_raw_to_voltage(adc1_get_raw(batteryPinADC), adc_chars) / 1000.0 * voltageDivider);
}   