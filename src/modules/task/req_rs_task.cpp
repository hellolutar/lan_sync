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
        LOG_INFO("ReqRsTask::sendRsReq() : req resource uri[{}{}] range[{}]!", ctx->getPeer().str().data(), uri, range_hdr);
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

    SyncRs rs = rsm.getAllUriRs()[uri];

    uint owner_size = rsm.getOwnerSize(uri);
    if (owner_size == 0 || rsm.getBlockSize(uri) == 0 || rsm.getSyncingSize(uri) > 10){
        LOG_INFO("ReqRsTask::run() : syncing!");
        return;
    }

    int num_block_per_owner = rsm.getBlockSize(uri) / owner_size + 1;
    LOG_INFO("ReqRsTask::run() : uri[{}]\towner_size[{}]\tnum_block_per_owner[{}]", uri, owner_size, num_block_per_owner);

    for (int i = 0; i < owner_size; i++)
    {
        NetAddr peer = rs.owner[i];
        for (int i = 0; i < num_block_per_owner; i++)
        {
            Block b = rsm.regReqSyncRsAuto(peer, uri);
            if (b.end == 0)
                goto quickbreak;

            auto ctx = idx[peer];

            sendRsReq(ctx, b);

            if (rsm.getSyncingSize(uri) > 10)
                goto quickbreak;
        }
    }
quickbreak:
    LOG_INFO("ReqRsTask::run() : done!");
    if (rsm.getBlockSize(uri) > 0)
    {
        TaskManager::getTaskManager()->addTask(new ReqRsTask(uri, uri));
    }
}