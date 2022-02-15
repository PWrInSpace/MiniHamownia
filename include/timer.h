#ifndef TIMER_HH
#define TIMER_HH

#include <stdint.h>
#include <Arduino.h>

class Timer{
    uint64_t timer;
    bool enable;

    public:
    Timer();
    void setTimer(uint64_t _timer);
    int64_t getTime() const;
    bool isEnable() const;
    void setDefault();
};


#endif