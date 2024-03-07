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

struct timeval &Trigger::getPeriod()
{
    return period;
}
bool &Trigger::getPersist()
{
    return persist;
}