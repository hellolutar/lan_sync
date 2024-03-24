#ifndef __BUF_BASE_ON_EVENT_H_
#define __BUF_BASE_ON_EVENT_H_

#include <event2/buffer.h>
#include "abs_buf.h"
#include "utils/logger.h"

class BufBaseonEvent : public AbsBuf
{
private:
    evbuffer *buf;

public:
    uint64_t add(uint8_t *data, uint64_t len) override;
    void *data() override;
    uint64_t size() override;
    BufBaseonEvent();
    ~BufBaseonEvent();
};



#endif