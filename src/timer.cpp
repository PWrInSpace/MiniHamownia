#include "timer.h"

Timer::Timer(): timer(0), enable(false){}

void Timer::setTimer(uint64_t _timer){
    timer = _timer;
    enable = true;
}

int64_t Timer::getTime() const{
    return (millis() - timer);
}

bool Timer::isEnable() const{
    return enable;
}

void Timer::setDefault(){
    timer = 0;
    enable = false;
}