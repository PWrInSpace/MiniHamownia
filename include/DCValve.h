#ifndef DC_VALVE_HH
#define DC_VALVE_HH

#include <stdint.h>
#include <Arduino.h>
#include <FreeRTOS.h>

class DCValve{
    const uint8_t motorPin1;
    const uint8_t motorPin2;
    const uint8_t pwmPin;
    const uint8_t limitSwitchPin1;
    const uint8_t limitSwitchPin2;
    uint16_t frequency;
    const uint8_t resolution;
    const uint8_t pwmChannel;
    uint16_t timeout;

    public:
    DCValve(uint8_t _motor1, uint8_t _motor2, uint8_t _pwm, uint8_t _lsp1, uint8_t _lsp2, uint16_t _freq = 1000, uint8_t _res = 8, uint8_t _chan = 0);
    void init();
    void valveMove(const uint8_t & limitSwitchPIN, const uint8_t & highValvePIN, const uint8_t & valveSpeed = 255);
    void setTimeout(uint16_t newTimeout);
    //Tasks
    void open(void *arg);  
    void close(void *arg);
    void timeOpen(void *arg);
};

#endif