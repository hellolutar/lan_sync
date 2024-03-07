#include "timer_trigger_with_event.h"

std::map<Trigger *, Trigger *> TimerWithEvent::triggers;
struct event_base *TimerWithEvent::base;

TimerWithEvent::~TimerWithEvent()
{
    for (auto iter = triggers.begin(); iter != triggers.end();)
    {
        auto entry = (*iter).first;
        iter = triggers.erase(iter);
        delete entry;
    }
}

static void timeout_cb(evutil_socket_t, short, void *arg)
{
    Trigger *tr = (Trigger *)arg;
    tr->exec();
}

void TimerWithEvent::init(struct event_base *eb)
{
    base = eb;
}

bool TimerWithEvent::addTr(TriggerWithEvent *tr)
{
    if (base == nullptr)
    {
        LOG_ERROR("please TimerWithEvent::init()");
        exit(-1);
        return false;
    }

    Trigger *exist = triggers[tr];
    if (exist != nullptr)
    {
        return false;
    }

    int flag = EV_TIMEOUT;
    if (tr->getPersist())
        flag |= EV_PERSIST;

    struct event *timeout = event_new(base, -1, flag, timeout_cb, tr);
    tr->setEvent(timeout);

    struct timeval &period = tr->getPeriod();

    event_add(timeout, &period);

    triggers[tr] = tr;

    return true;
}

bool TimerWithEvent::delTr(Trigger *tr)
{
    for (auto iter = triggers.begin(); iter != triggers.end(); iter++)
    {
        if (tr == (*iter).first)
        {
            triggers.erase(iter);
            delete (*iter).first;
            return true;
        }
    }

    return false;
}

void TimerWithEvent::run()
{
    event_base_dispatch(base);
}

TriggerWithEvent::~TriggerWithEvent()
{
    if (ev != nullptr)
        event_free(ev);
}

void TriggerWithEvent::setEvent(struct event *ev)
{
    this->ev = ev;
}