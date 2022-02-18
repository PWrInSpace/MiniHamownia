#ifndef HX711_HH
#define HX711_HH
#include "HX711_ADC.h"

class HX711 : public HX711_ADC
{
    public:
    HX711(uint8_t dout, uint8_t sck) : HX711_ADC(dout, sck) {};
    float regressionCalibration();
};

#endif