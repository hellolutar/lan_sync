#include "net_cli_conn_setup_for_discover.h"

using namespace std;
NetCliConnSetupForDiscover::NetCliConnSetupForDiscover(/* args */)
{
}

NetCliConnSetupForDiscover::~NetCliConnSetupForDiscover()
{
}

NetCliLogicContainer *NetCliConnSetupForDiscover::setupConn(NetAddr peer, AbstNetLogic &logic)
{
    // LOG_INFO("NetCliConnSetupForDiscover setupConn: {}",)
    NetCliLogicContainer *udpcli = new NetCliLogicContainer(peer, logic);                     // ne 何时释放
    NetworkConnCtx *nctx = NetFrameworkImplWithEvent::connectWithUdp(udpcli); // nctx 何时释放: 由NetworkLayerWithEvent负责释放
    udpcli->setCtx(nctx);
    return udpcli;
}

void NetCliConnSetupForDiscover::exec(NetworkConnCtx &ctx)
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
