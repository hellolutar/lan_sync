#include "sync_trigger.h"

using namespace std;

SyncCliDiscoverLogic::SyncCliDiscoverLogic(/* args */)
{
    this->name = MODULE_NAME_DISCOVER_LOGIC;
};

void SyncCliDiscoverLogic::mod_conn_recv(std::string from, std::string uri, void *data)
{
    throw "SyncCliSyncLogic::mod_conn_recv() : unsupport";
}

void SyncCliDiscoverLogic::exec(shared_ptr<NetworkConnCtx> &ctx)
{
    LanSyncPkt pkt(lan_sync_version::VER_0_1, LAN_SYNC_TYPE_HELLO);

    BufBaseonEvent buf;
    pkt.write(buf);

    LOG_DEBUG("SyncCliDiscoverLogic::exec() : send HELLO");
    try
    {
        ctx->write(buf.data(), buf.size());
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        LOG_ERROR("SyncCliDiscoverLogic::exec() : reason:{}", e.what());
        mod_conn_send(MODULE_NAME_DISCOVERY, MODULE_CONN_URI_DISCOVER_DEL, &ctx); // ctx由DiscoveryTrigger#delNetAddr --> delete udpcli负责释放
    }
}

static SyncCliDiscoverLogic discovery_logic;

DiscoveryTrigger::DiscoveryTrigger(struct timeval period, bool persist, AbstNetLogic &net_recv_logic, AbsModConnMediator *med)
    : UdpTrigger(period, persist, net_recv_logic, discovery_logic)
{
    this->med = med;
    this->name = MODULE_NAME_DISCOVERY;
    discovery_logic.setMediator(med);
    med->add(&discovery_logic);
}

DiscoveryTrigger::~DiscoveryTrigger()
{
}

void DiscoveryTrigger::mod_conn_recv(std::string from, std::string uri, void *data)
{
    LOG_DEBUG("DiscoveryTrigger::mod_conn_recv() : from:{},\turi:{}", from, uri);

    if (StringUtils::eq(MODULE_CONN_URI_DISCOVER_ADD, uri))
    {
        NetworkConnCtx *ctx = (NetworkConnCtx *)data;
        addConn(ctx->getPeer());
    }
    else if (StringUtils::eq(MODULE_CONN_URI_DISCOVER_DEL, uri))
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

void SyncCliSyncLogic::mod_conn_recv(std::string from, std::string uri, void *data)
{
    throw "SyncCliSyncLogic::mod_conn_recv() : unsupport";
}

void SyncCliSyncLogic::reqTbIdx(shared_ptr<NetworkConnCtx> &ctx)
{
    LanSyncPkt pkt(lan_sync_version::VER_0_1, LAN_SYNC_TYPE_GET_TABLE_INDEX);

    BufBaseonEvent buf;
    pkt.write(buf);

    LOG_DEBUG("SyncCliSyncLogic::reqTbIdx() : send GET_TABBLE_IDX");
    try
    {
        ctx->write(buf.data(), buf.size()); // try catch in netrigger
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        LOG_ERROR("SyncCliSyncLogic::reqTbIdx() : reason:{}", e.what());
        mod_conn_send(MODULE_NAME_PERIOD_REQ_TB_IDX, MODULE_CONN_URI_PERIOD_REQ_TB_IDX_DEL, &ctx); // ctx由DiscoveryTrigger#delNetAddr --> delete udpcli负责释放
    }
}

void SyncCliSyncLogic::exec(shared_ptr<NetworkConnCtx> &ctx)
{
    reqTbIdx(ctx);
}

static SyncCliSyncLogic req_tb_idx_logic;
SyncReqTbIdxTrigger::SyncReqTbIdxTrigger(struct timeval period, bool persist, AbstNetLogic &net_recv_logic, AbsModConnMediator *med)
    : TcpTrigger(period, persist, net_recv_logic, req_tb_idx_logic)
{
    this->med = med;
    this->name = MODULE_NAME_PERIOD_REQ_TB_IDX;
    req_tb_idx_logic.setMediator(med);
    med->add(&req_tb_idx_logic);
}

SyncReqTbIdxTrigger::~SyncReqTbIdxTrigger()
{
}

void SyncReqTbIdxTrigger::mod_conn_recv(std::string from, std::string uri, void *data)
{
    LOG_DEBUG("SyncReqTbIdxTrigger::mod_conn_recv() : from:{},\turi:{}", from, uri);

    if (StringUtils::eq(MODULE_CONN_URI_PERIOD_REQ_TB_IDX_ADD, uri))
    {
        NetAddr *addr = (NetAddr *)data;
        addConn(*addr);
    }
    else if (StringUtils::eq(MODULE_CONN_URI_PERIOD_REQ_TB_IDX_DEL, uri))
    {
        shared_ptr<NetworkConnCtx> ctx = *(shared_ptr<NetworkConnCtx> *)data;
        delNetAddr(ctx->getPeer());
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
