#ifndef __SYNC_IO_READ_MONITOR_H_
#define __SYNC_IO_READ_MONITOR_H_

#include <cstdio>

#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "utils/io_utils.h"
#include "proto/lan_share_protocol.h"
#include "net/net_framework_impl_with_event.h"
#include "vo/dto/resource.h"
#include "components/buf_base_on_event.h"


class SyncIOReadMonitor : public IoReadMonitor
{
private:
    NetworkConnCtx *ctx;
    const struct Resource *rs;

public:
    SyncIOReadMonitor(NetworkConnCtx *ctx, const struct Resource *rs) : ctx(ctx), rs(rs){};
    ~SyncIOReadMonitor(){};
    void monitor(uint64_t from_pos, void *data, uint64_t data_len) override;
};

#endif
