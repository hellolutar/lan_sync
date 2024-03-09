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
