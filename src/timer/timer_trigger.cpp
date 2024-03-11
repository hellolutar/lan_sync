#include "timer_trigger.h"

using namespace std;
Timer::Timer(/* args */)
{
}

Timer::~Timer(/* args */)
{
}

Trigger::~Trigger()
{
}

timeval Trigger::second(size_t s)
{
    struct timeval tv;
    tv.tv_usec = 0;
    tv.tv_sec = s;
    return tv;
}

struct timeval &Trigger::getPeriod()
{
    return period;
}
bool &Trigger::getPersist()
{
    return persist;
}