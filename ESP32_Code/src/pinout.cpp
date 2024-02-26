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

void pinInit()
{
    adc_power_acquire();
    pinMode(IGNITER, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    //pinMode(BATT_CHECK, INPUT);
    //pinMode(CONTINUITY, INPUT);
    //pinMode(BATT_CHECK, INPUT);
    //pinMode(CONTINUITY, INPUT);
    digitalWrite(IGNITER, LOW);
    digitalWrite(BUZZER, LOW);
    //
}

