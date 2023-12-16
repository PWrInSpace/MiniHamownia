#include "pinout.h"


void beepBoop(int time, int howManyTimes){
    uint32_t delay = time / howManyTimes;
    for(int i=0; i < howManyTimes; ++i){
        digitalWrite(BUZZER, 1);
        vTaskDelay(delay / portTICK_PERIOD_MS);
        digitalWrite(BUZZER, 0);
        vTaskDelay(delay / portTICK_PERIOD_MS);
    }
}

// reverseDividerVal = (R1 + R2) / R2
float checkBattery(uint8_t batteryPin, const float &reverseDividerVal)
{
    return analogRead(batteryPin) / 1575.0 * 1.07 * reverseDividerVal;
}

void pinInit()
{
    //
    pinMode(IGNITER, OUTPUT); // igniter
    digitalWrite(IGNITER, LOW);
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);
    analogReadResolution(12);
    analogSetWidth(12);
    analogSetAttenuation(ADC_11db);
    pinMode(BATT_CHECK, INPUT);
    pinMode(CONTINUITY, INPUT);
    pinMode(PRESS_SENS, INPUT);
    //
}

