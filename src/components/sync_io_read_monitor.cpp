#include "sync_io_read_monitor.h"

using namespace std;


void SyncIOReadMonitor::monitor(uint64_t from_pos, void *data, uint64_t data_len)
{
    LanSyncPkt reply_pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_RESOURCE);

    reply_pkt.addXheader(XHEADER_HASH, rs->hash);
    reply_pkt.addXheader(XHEADER_URI, rs->uri);

    if (rs->size == from_pos + data_len)
    {
        ContentRange cr(from_pos, data_len, rs->size, true);
        reply_pkt.addXheader(XHEADER_CONTENT_RANGE, cr.to_string());
        LOG_INFO("SyncIOReadMonitor::monitor : [{}] {}", rs->uri, cr.to_string());
    }
    else
    {
        ContentRange cr(from_pos, data_len, rs->size, false);
        reply_pkt.addXheader(XHEADER_CONTENT_RANGE, cr.to_string());
        LOG_INFO("SyncIOReadMonitor::monitor : [{}] {}", rs->uri, cr.to_string());
    }

    reply_pkt.setData(data, data_len);

    struct evbuffer *buf = evbuffer_new();
    reply_pkt.write(buf);

    uint64_t reply_data_len = evbuffer_get_length(buf);
    uint8_t *reply_data = (uint8_t *)malloc(reply_data_len);
    evbuffer_remove(buf, reply_data, reply_data_len);
    evbuffer_free(buf);
    ctx->write(reply_data, reply_data_len);
    free(reply_data);
}