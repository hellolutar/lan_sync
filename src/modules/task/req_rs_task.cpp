#include "req_rs_task.h"

using namespace std;

std::default_random_engine e;

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
    LOG_DEBUG("ReqRsTask::run() : start!");

    map<NetAddr, NetworkConnCtx *> idx = NetFrameworkImplWithEvent::getAllTcpSession();

    RsSyncManager &rsm = ResourceManager::getRsSyncManager();

    SyncRs rs = rsm.getAllUriRs()[uri];

    uint owner_size = rsm.getOwnerSize(uri);
    if (owner_size == 0 || rsm.getBlockSize(uri) == 0 || rsm.getSyncingSize(uri) >= DOWNLOAD_LIMIT)
    {
        LOG_DEBUG("ReqRsTask::run() : syncing!");
        return;
    }

    int num_block_per_owner = rsm.getBlockSize(uri) / owner_size + 1;
    LOG_DEBUG("ReqRsTask::run() : uri[{}]\towner_size[{}]\tnum_block_per_owner[{}]", uri, owner_size, num_block_per_owner);

    std::uniform_int_distribution<int> u(0, owner_size - 1); // 左闭右闭区间
    e.seed(time(0));
    int who = u(e);

    NetAddr peer = rs.owner[who];
    Block b = rsm.regReqSyncRsAuto(peer, uri);
    if (b.end != 0)
    {
        auto ctx = idx[peer];
        sendRsReq(ctx, b);
    }
}