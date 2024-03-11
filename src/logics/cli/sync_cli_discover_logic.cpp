#include "sync_cli_discover_logic.h"

SyncCliDiscoverLogic::SyncCliDiscoverLogic(/* args */){
};

void SyncCliDiscoverLogic::exec(NetworkConnCtx &ctx)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_HELLO);
    struct evbuffer *buf = evbuffer_new();
    pkt.write(buf);

    uint64_t data_len = evbuffer_get_length(buf);
    uint8_t *data = (uint8_t *)malloc(data_len);
    evbuffer_remove(buf, data, data_len);
    ctx.write(data, data_len);

    evbuffer_free(buf);
    free(data);
}