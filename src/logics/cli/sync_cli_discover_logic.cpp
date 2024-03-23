#include "sync_cli_discover_logic.h"

SyncCliDiscoverLogic::SyncCliDiscoverLogic(/* args */){};

void SyncCliDiscoverLogic::exec(NetworkConnCtx &ctx)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_HELLO);

    BufBaseonEvent buf;
    pkt.write(buf);

    LOG_DEBUG("SyncCliDiscoverLogic::exec() : send HELLO");
    ctx.write(buf.data(), buf.size()); // try catch in netrigger
}