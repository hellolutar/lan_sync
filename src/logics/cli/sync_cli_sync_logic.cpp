#include "sync_cli_sync_logic.h"

void SyncCliSyncLogic::reqTbIdx(NetworkConnCtx &ctx)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_TABLE_INDEX);

    BufBaseonEvent buf;
    pkt.write(buf);
    ctx.write(buf.data(), buf.size());
}

void SyncCliSyncLogic::reqRs(NetworkConnCtx &ctx)
{
    uint8_t max_concur = 10;
    int count = 0;
    RsSyncManager &rsm = ResourceManager::getRsSyncManager();
    for (auto uriRs : rsm.getAllUriRs())
    {
        if (count >= max_concur)
            break;

        Block b = rsm.regReqSyncRsAuto(ctx.getPeer(), uriRs.first);
        if (b.end == 0)
            continue;

        sendReqRs(ctx, uriRs.first, b);
    }
}

void SyncCliSyncLogic::sendReqRs(NetworkConnCtx &ctx, string uri, Block b)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_RESOURCE);

    string range_hdr = Range(b.start, b.end - b.start).to_string();
    pkt.addXheader(XHEADER_URI, uri);
    pkt.addXheader(XHEADER_RANGE, range_hdr);

    BufBaseonEvent buf;
    pkt.write(buf);
    ctx.write(buf.data(), buf.size());

    LOG_INFO("[SYNC CLI] req resource uri[{}] range[{}]!", uri, range_hdr);
}

void SyncCliSyncLogic::exec(NetworkConnCtx &ctx)
{
    reqRs(ctx);
    reqTbIdx(ctx);
}