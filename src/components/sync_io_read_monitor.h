#ifndef __SYNC_IO_READ_MONITOR_H_
#define __SYNC_IO_READ_MONITOR_H_

#include <cstdio>

#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "utils/io_utils.h"
#include "proto/lan_share_protocol.h"
#include "net/network_layer_with_event.h"
#include "resource/resource.h"

class SyncIOReadMonitor : public IoReadMonitor
{
private:
    struct bufferevent *bev;
    const struct Resource *rs;

public:
    SyncIOReadMonitor(struct bufferevent *bev, const struct Resource *rs);
    ~SyncIOReadMonitor();
    void monitor(uint64_t from_pos, void *data, uint64_t data_len) override;
};

class SyncIOReadMonitor2 : public IoReadMonitor
{
private:
    NetworkConnCtx *ctx;
    const struct Resource *rs;

public:
    SyncIOReadMonitor2(NetworkConnCtx *ctx, const struct Resource *rs) : ctx(ctx), rs(rs){};
    ~SyncIOReadMonitor2();
    void monitor(uint64_t from_pos, void *data, uint64_t data_len) override;
};

#endif
