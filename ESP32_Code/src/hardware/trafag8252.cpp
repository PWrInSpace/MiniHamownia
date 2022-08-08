#include "../include/hardware/trafag8252.h"

Trafag8252::Trafag8252(uint8_t _pressurePinADC, uint16_t _calibrationConstant) : 
    pressurePinADC(_pressurePinADC), 
    calibrationConstant(_calibrationConstant) {pinMode(pressurePinADC, INPUT);}
 
float Trafag8252::getPressure()
{
    return analogRead(pressurePinADC) * calibrationConstant;
}

float Trafag8252::sensorCalibration(const float &pressure)
{
    uint16_t measuredVoltage = analogRead(pressurePinADC);
    return pressure / ((float) measuredVoltage);
}