#include "timer/timer_trigger_with_event.h"
#include <chrono>
#include <sstream>

class HelloTrigger : public TriggerWithEvent
{
private:
    int count;

public:
    HelloTrigger(struct timeval period, bool persist) : TriggerWithEvent(period, persist){};
    ~HelloTrigger(){};
    void exec()
    {
        time_t t;
        time(&t);
        std::tm tm = *std::localtime(&t);
        // std::tm tm = *std::gmtime(&t);
        std::stringstream ss;
        ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        printf("%s : hello world\n", ss.str().data());

        count++;
        if (count >= 5)
            delete this;
    }
};

int main(int argc, char const *argv[])
{
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 1;
    TriggerWithEvent *tr = new HelloTrigger(tv, true);

    struct event_base *base = event_base_new();
    TimerWithEvent::init(base);
    TimerWithEvent::addTr(tr);
    TimerWithEvent::run();
    event_base_free(base);

    return 0;
}
