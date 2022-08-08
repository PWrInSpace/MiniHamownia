#ifndef DC_VALVE_HH
#define DC_VALVE_HH

#include <FreeRTOS.h>
#include <Arduino.h>
#include <stdint.h>
#include "pinout.h"

//DCValve secondValve(DCIN1, DCIN2, DC_PWM1, LIM_SW_1, LIM_SW_2);

class DCValve{
    const uint8_t motorPin1;
    const uint8_t motorPin2;
    const uint8_t pwmPin;
    const uint8_t limitSwitchPin1;
    const uint8_t limitSwitchPin2;
    uint16_t frequency;
    const uint8_t resolution;
    const uint8_t pwmChannel;
    const uint16_t timeout;

    public:
    DCValve(uint8_t _motor1, uint8_t _motor2, uint8_t _pwm, uint8_t _lsp1, uint8_t _lsp2, uint16_t _freq = 1000, uint8_t _res = 8, uint8_t _chan = 0);
    void init();
    void valveMove(const uint8_t & limitSwitchPIN, const uint8_t & highValvePIN, const uint8_t & valveSpeed = 255);
    //void setTimeout(uint16_t newTimeout);
    //Tasks
    void open();  
    void close();
    void timeOpen(uint32_t time);
    String getPosition();
};

#ifdef MAIN_FREERTOS_H_

void openFirstValve(void *arg);
void openSecondValve(void *arg);
void closeFirstValve(void *arg);
void closeSecondValve(void *arg);
void timeOpenFirstValve(void  *arg);
void timeOpenSecondValve(void  *arg);

#endif



#endif