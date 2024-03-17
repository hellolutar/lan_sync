#include "req_rs_task.h"

using namespace std;

void ReqRsTask::sendRsReq(NetworkConnCtx *ctx, Block b)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_RESOURCE);

    string range_hdr = Range(b.start, b.end - b.start).to_string();
    pkt.addXheader(XHEADER_URI, uri);
    pkt.addXheader(XHEADER_RANGE, range_hdr);

    BufBaseonEvent buf;
    pkt.write(buf);
    NetAddr peer = ctx->getPeer();
    try
    {
        ctx->write(buf.data(), buf.size());
        LOG_INFO("ReqRsTask::sendRsReq() : req resource uri[{}] range[{}]!", uri, range_hdr);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        ResourceManager::getRsSyncManager().unregAllReqSyncRsByPeer(peer, uri);
        delete ctx;
        ctx = nullptr;
    }
}

void ReqRsTask::run()
{
    LOG_INFO("ReqRsTask::run() : start!");

    map<NetAddr, NetworkConnCtx *> idx = NetFrameworkImplWithEvent::getAllTcpSession();

    RsSyncManager &rsm = ResourceManager::getRsSyncManager();

    for (auto uriRs : rsm.getAllUriRs())
    {
        string uri = uriRs.first;
        SyncRs rs = uriRs.second;

        int num_block_per_owner = rs.owner.size() / rs.block.size() + 1;
        LOG_INFO("ReqRsTask::run() : uri[{}]  owern[{}] num_block_per_owner[{}]", uri, rs.owner.size(), num_block_per_owner);

        for (int i = 0; i < rs.owner.size(); i++)
        {
            NetAddr peer = rs.owner[i];
            for (int i = 0; i < num_block_per_owner; i++)
            {
                Block b = rsm.regReqSyncRsAuto(peer, uri);
                if (b.end == 0)
                    goto quickbreak;

                auto ctx = idx[peer];

                sendRsReq(ctx, b);
            }
        }
    quickbreak:
        LOG_INFO("ReqRsTask::run() : quickbreak!");
    }
    LOG_INFO("ReqRsTask::run() : done!");
}