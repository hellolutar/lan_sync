#ifndef __SIMPLE_TRIGGER_H_
#define __SIMPLE_TRIGGER_H_

#include "timer/timer_trigger_with_event.h"
#include <chrono>
#include <sstream>

class SimpleTrigger : public TriggerWithEvent
{
private:
    int count;

public:
    SimpleTrigger(struct timeval period, bool persist) : TriggerWithEvent(period, persist){};
    ~SimpleTrigger(){};
    void exec() override;
};

#endif