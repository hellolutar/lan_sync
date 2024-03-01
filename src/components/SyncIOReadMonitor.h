#ifndef __SyncIOReadMonitor_H_
#define __SyncIOReadMonitor_H_

#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "utils/io_utils.h"
#include "comm/lan_share_protocol.h"
#include "resource/resource.h"

class SyncIOReadMonitor : public IoReadMonitor
{
private:
    struct bufferevent *bev;
    const struct Resource *rs;

public:
    SyncIOReadMonitor(struct bufferevent *bev, const struct Resource *rs) ;
    ~SyncIOReadMonitor();
    void monitor(uint64_t from_pos, void *data, uint64_t data_len) override;
};

#endif
