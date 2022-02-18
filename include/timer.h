#ifndef TIMER_HH
#define TIMER_HH

#include <stdint.h>
#include <Arduino.h>

class Timer{
    uint32_t timer;
    bool enable;

    public:
    Timer();
    void setTimer(uint32_t _timer);
    int32_t getTime() const;
    bool isEnable() const;
    void setDefault();
};


#endif