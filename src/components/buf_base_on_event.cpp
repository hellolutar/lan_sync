#include "buf_base_on_event.h"

BufBaseonEvent::BufBaseonEvent(/* args */)
{
    buf = evbuffer_new();
}

BufBaseonEvent::~BufBaseonEvent()
{
     if (buf != nullptr)
        evbuffer_free(buf);
}

uint64_t BufBaseonEvent::add(uint8_t *data_in, uint64_t len)
{
    if (evbuffer_add(buf, data_in, len) == -1)
    {
        return 0;
    }
    return len;
}

void *BufBaseonEvent::data()
{
    if (d != nullptr)
    {
        delete[] d;
    }

    uint64_t len = evbuffer_get_length(buf);
    d = new uint8_t[len];
    uint64_t act_len = evbuffer_copyout(buf, d, len);
    if (act_len != len)
    {
        LOG_WARN("BufBaseonEvent::data():  act_len != len : {} != {}", act_len, len);
    }
    return d;
}

uint64_t BufBaseonEvent::size()
{
    return  evbuffer_get_length(buf);
}
