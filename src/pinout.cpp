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