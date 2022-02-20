#include "pinout.h"

void beepBoop(int delay, int times)
{
    for (int i = 0; i < times; ++i)
    {
        digitalWrite(BUZZER, 1);
        vTaskDelay(delay / portTICK_PERIOD_MS);
        digitalWrite(BUZZER, 0);
        vTaskDelay(delay / portTICK_PERIOD_MS);
    }
}

// reverseDividerVal = (R1 + R2) / R2
float checkBattery(uint8_t batteryPin, const float &reverseDividerVal)
{
    return analogRead(batteryPin) * reverseDividerVal;
}

void pinInit()
{
    //
    pinMode(2, OUTPUT); // debug led
    digitalWrite(2, HIGH);
    pinMode(IGNITER, OUTPUT); // igniter
    digitalWrite(IGNITER, LOW);
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);
    //
}