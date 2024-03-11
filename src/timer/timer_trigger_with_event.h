#ifndef __TIMER_TRIGGER_WITH_EVENT_H_
#define __TIMER_TRIGGER_WITH_EVENT_H_

#include <event2/event.h>

#include "utils/logger.h"
#include "timer_trigger.h"

class TriggerWithEvent;

class TimerWithEvent : public Timer
{
    static std::map<Trigger *, Trigger *> triggers;

private:
    static struct event_base *base;
    bool addTrigger(Trigger *tr) override {return false;};
    bool rmTrigger(Trigger *tr) override { return false; };

public:
    virtual ~TimerWithEvent();

    static void init(struct event_base *eb);
    static bool addTr(TriggerWithEvent *tr);
    static bool delTr(Trigger *tr);
    static void run();
};

class TriggerWithEvent : public Trigger
{
private:
    struct event *ev;

protected:
public:
    TriggerWithEvent(struct timeval period, bool persist) : Trigger(period, persist){};
    virtual ~TriggerWithEvent();
    void setEvent(struct event *);
};

#endif