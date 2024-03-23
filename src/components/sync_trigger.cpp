#include "sync_trigger.h"

using namespace std;

SyncCliDiscoverLogic::SyncCliDiscoverLogic(/* args */){};

void SyncCliDiscoverLogic::exec(NetworkConnCtx &ctx)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_HELLO);

    BufBaseonEvent buf;
    pkt.write(buf);

    LOG_DEBUG("SyncCliDiscoverLogic::exec() : send HELLO");
    ctx.write(buf.data(), buf.size()); // try catch in netrigger
}

static SyncCliDiscoverLogic discovery_logic;

DiscoveryTrigger::DiscoveryTrigger(struct timeval period, bool persist, AbstNetLogic &net_recv_logic, AbsModConnMediator *med)
    : UdpTrigger(period, persist, net_recv_logic, discovery_logic)
{
    this->med = med;
    this->name = MODULE_NAME_DISCOVERY;
}

DiscoveryTrigger::~DiscoveryTrigger()
{
}

void DiscoveryTrigger::mod_conn_recv(std::string from, std::string uri, void *data)
{
    LOG_DEBUG("DiscoveryTrigger::mod_conn_recv() : from:{},\turi:{}", from, uri);

    if (uri.find("add") != string::npos)
    {
        NetworkConnCtx *ctx = (NetworkConnCtx *)data;
        addConn(ctx->getPeer());
    }
    else if (uri.find("del") != string::npos)
    {
        NetworkConnCtx *ctx = (NetworkConnCtx *)data;
        delNetAddr(ctx->getPeer());
    }
    else
    {
        StringBuilder sb;
        sb.add("DiscoveryTrigger::mod_conn_recv() : unsupport uri:");
        sb.add(uri);

        string errorMsg = sb.str();
        LOG_ERROR(errorMsg);
    }
}

void SyncCliSyncLogic::reqTbIdx(NetworkConnCtx &ctx)
{
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_TABLE_INDEX);

    BufBaseonEvent buf;
    pkt.write(buf);

    LOG_DEBUG("SyncCliSyncLogic::reqTbIdx() : send GET_TABBLE_IDX");
    ctx.write(buf.data(), buf.size()); // try catch in netrigger
}

void SyncCliSyncLogic::exec(NetworkConnCtx &ctx)
{
    reqTbIdx(ctx);
}

static SyncCliSyncLogic req_tb_idx_logic;
SyncReqTbIdxTrigger::SyncReqTbIdxTrigger(struct timeval period, bool persist, AbstNetLogic &net_recv_logic, AbsModConnMediator *med)
    : TcpTrigger(period, persist, net_recv_logic, req_tb_idx_logic)
{
    this->med = med;
    this->name = MODULE_NAME_PERIOD_REQ_TB_IDX;
}

SyncReqTbIdxTrigger::~SyncReqTbIdxTrigger()
{
}

void SyncReqTbIdxTrigger::mod_conn_recv(std::string from, std::string uri, void *data)
{
    LOG_DEBUG("SyncReqTbIdxTrigger::mod_conn_recv() : from:{},\turi:{}", from, uri);

    if (uri.find("add") != string::npos)
    {
        NetAddr *addr = (NetAddr *)data;
        addConn(*addr);
    }
    else if (uri.find("del") != string::npos)
    {
        NetAddr *addr = (NetAddr *)data;
        delNetAddr(*addr);
    }
    else
    {
        StringBuilder sb;
        sb.add("NetTrigger::mod_conn_recv() : unsupport uri:");
        sb.add(uri);

        string errorMsg = sb.str();
        LOG_ERROR(errorMsg);
    }
}
