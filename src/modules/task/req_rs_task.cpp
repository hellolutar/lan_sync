#include "req_rs_task.h"

using namespace std;

void ReqRsTask::run()
{

    for (auto &b : reqBlocks)
    {
        LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_RESOURCE);

        string range_hdr = Range(b.start, b.end - b.start).to_string();
        pkt.addXheader(XHEADER_URI, uri);
        pkt.addXheader(XHEADER_RANGE, range_hdr);

        BufBaseonEvent buf;
        pkt.write(buf);
        NetAddr peer = nctx->getPeer();
        try
        {
            nctx->write(buf.data(), buf.size());
            LOG_INFO("[SYNC CLI] req resource uri[{}] range[{}]!", uri, range_hdr);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            ResourceManager::getRsSyncManager().unregAllReqSyncRsByPeer(peer, uri);
            break;
        }
    }
}