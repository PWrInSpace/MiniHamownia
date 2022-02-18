#include "pinout.h"

void beepBoop(int delay, int times){
    for(int i=0; i < times; ++i){
        digitalWrite(BUZZER, 1);
        vTaskDelay(delay / portTICK_PERIOD_MS);
        digitalWrite(BUZZER, 0);
        vTaskDelay(delay / portTICK_PERIOD_MS);
    }
}