#include "sync_cli_sync_logic.h"

void SyncCliSyncLogic::reqTbIdx(NetworkConnCtx &ctx)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_TABLE_INDEX);

    BufBaseonEvent buf;
    pkt.write(buf);
    ctx.write(buf.data(), buf.size());
}

void SyncCliSyncLogic::exec(NetworkConnCtx &ctx)
{
    reqTbIdx(ctx);
}