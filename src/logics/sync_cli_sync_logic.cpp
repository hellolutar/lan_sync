#include "sync_cli_sync_logic.h"

void SyncCliReqTbIdxLogic::exec(NetworkConnCtx &ctx)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_TABLE_INDEX);

    struct evbuffer *buf = evbuffer_new();
    pkt.write(buf);

    uint64_t reply_data_len = evbuffer_get_length(buf);
    uint8_t *reply_data = (uint8_t *)malloc(reply_data_len);
    evbuffer_remove(buf, reply_data, reply_data_len);
    evbuffer_free(buf);
    ctx.write(reply_data, reply_data_len);
    free(reply_data);
}

void SyncCliReqRsLogic::hdl_pending(NetworkConnCtx &ctx, WantToSyncVO vo)
{
    // TODO check tcp session exist
    ResourceManager::getRsm().updateSyncEntryStatus(vo.getUri(), PENDING);

    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_RESOURCE);

    string range_hdr = vo.getRange().to_string();
    pkt.addXheader(XHEADER_URI, vo.getUri());
    pkt.addXheader(XHEADER_RANGE, range_hdr);

    struct evbuffer *buf = evbuffer_new();
    pkt.write(buf);

    uint64_t reply_data_len = evbuffer_get_length(buf);
    uint8_t *reply_data = (uint8_t *)malloc(reply_data_len);
    evbuffer_remove(buf, reply_data, reply_data_len);
    evbuffer_free(buf);
    ctx.write(reply_data, reply_data_len);
    free(reply_data);

    LOG_INFO("[SYNC CLI] req resource uri[{}] range[{}]!", vo.getUri().data(), range_hdr.data());
}

void SyncCliReqRsLogic::hdl_syncing(WantToSyncVO vo)
{
    ResourceManager &rsm = ResourceManager::getRsm();

    time_t now;
    time(&now);
    int diff = difftime(now, vo.getLast_update_time());
    if (diff > vo.getMaxDelay())
    {
        LOG_WARN("[SYNC CLI] : uri[{}] cost a lot of time! now reset status", vo.getUri().data());

        rsm.updateSyncEntryStatus(vo.getUri(), PENDING);
        rsm.updateSyncEntryLastUpteTime(vo.getUri(), now);
    }
}

void SyncCliReqRsLogic::exec(NetworkConnCtx &ctx)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_RESOURCE);
    struct evbuffer *buf = evbuffer_new();

    ResourceManager &rsm = ResourceManager::getRsm();
    map<string, WantToSyncVO> &syncTb = rsm.getSyncTable();

    uint8_t max_concur = 10;
    int count = 0;
    for (auto iter = syncTb.begin(); iter != syncTb.end(); iter++)
    {
        if (count >= max_concur)
            break;

        WantToSyncVO vo = iter->second;
        time_t now;
        time(&now);

        switch (vo.getStatus())
        {
        case PENDING:
            count++;
            hdl_pending(ctx, vo);
            break;
        case SYNCING:
            hdl_syncing(vo);
            break;
        case SUCCESS:
            LOG_INFO("[SYNC CLI] : uri[{}] sync success! ", vo.getUri().data());
            break;
        case FAIL:
            rsm.updateSyncEntryStatus(vo.getUri(), PENDING);
            rsm.updateSyncEntryLastUpteTime(vo.getUri(), now);
            LOG_INFO("[SYNC CLI] : uri[{}] sync fail!  now reset status", vo.getUri().data());
            break;
        default:
            LOG_WARN(" [SYNC CLI] : uri[{}] sync status is unsupport", vo.getUri().data());
            break;
        }
    }
}