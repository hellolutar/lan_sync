#ifndef __TIMER_TRIGGER_H_
#define __TIMER_TRIGGER_H_

#include <map>
#include <ctime>

class Trigger;

class Timer
{
protected:
public:
    Timer(/* args */);
    virtual bool addTrigger(Trigger *tr) = 0;
    virtual bool rmTrigger(Trigger *tr) = 0;
    virtual ~Timer();
};

class Trigger
{
protected:
    struct timeval period;

    bool persist;

public:
    Trigger(struct timeval period, bool persist) : period(period), persist(persist){};
    virtual ~Trigger();
    virtual void exec() = 0;

    virtual struct timeval &getPeriod();
    virtual bool &getPersist();

    static timeval second(size_t s);
};

#endif
