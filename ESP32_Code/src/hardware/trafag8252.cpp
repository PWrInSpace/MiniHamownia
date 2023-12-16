#include "../include/hardware/trafag8252.h"

Trafag8252::Trafag8252(uint8_t _pressurePinADC) : 
    pressurePinADC(_pressurePinADC) {}
 
float Trafag8252::getPressure()
{
    //return (analogRead(pressurePinADC) / 1575.0) * 1.5;// * ((4498.0 + 9200.0) / 4498.0)); //Output w barach
    return ((analogReadMilliVolts(pressurePinADC) / 1000.0 * (4498.0 + 9200.0) / 4498.0) - 0.5) * 17.5;
}   