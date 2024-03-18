#include "sync_io_read_monitor.h"

using namespace std;

void SyncIOReadMonitor::monitor(uint64_t from_pos, void *data, uint64_t data_len)
{
    LanSyncPkt reply_pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_RESOURCE);

    reply_pkt.addXheader(XHEADER_HASH, rs->hash);
    reply_pkt.addXheader(XHEADER_URI, rs->uri);

    if (range.getStartPos() + range.getSize() == from_pos + data_len)
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

    BufBaseonEvent buf;
    reply_pkt.write(buf);

    ctx.write(buf.data(), buf.size());
}