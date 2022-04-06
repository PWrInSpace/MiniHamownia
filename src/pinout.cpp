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
    return (analogRead(batteryPin) / 4095.0   ) * 3.3 * reverseDividerVal * 8.0 / 7.0;
}

void pinInit()
{
    //
    pinMode(IGNITER, OUTPUT); // igniter
    digitalWrite(IGNITER, LOW);
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);
    pinMode(BATT_CHECK, INPUT);
    //
}