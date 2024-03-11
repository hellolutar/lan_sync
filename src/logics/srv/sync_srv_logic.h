#ifndef __SYNC_SRV_LOGIC_H_
#define __SYNC_SRV_LOGIC_H_

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <string>
#include <vector>
#include <algorithm>

#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "constants.h"
#include "utils/logger.h"
#include "utils/io_utils.h"
#include "modules/resource_manager.h"
#include "net/abst_net_logic.h"
#include "proto/lan_share_protocol.h"
#include "components/sync_io_read_monitor.h"

class SyncSrvLogic : public LogicTcp, public LogicUdp
{
public:
    enum state st;
    SyncSrvLogic();
    ~SyncSrvLogic(){};
    void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len) override;
    void replyTableIndex(NetworkConnCtx *ctx);
    void replyResource(lan_sync_header_t *header, NetworkConnCtx *ctx);
};

#endif