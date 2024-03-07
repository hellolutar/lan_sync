#include "discovery_trigger.h"

using namespace std;

DiscoverUdpCli::~DiscoverUdpCli()
{
    net_ctx->destroy();
}

void DiscoverUdpCli::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    printf("recv: %s \n", (char *)data);
}
bool DiscoverUdpCli::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return data_len >= LEN_LAN_SYNC_HEADER_T;
}

void DiscoverUdpCli::setCtx(NetworkConnCtx * ctx){
    this->net_ctx = ctx;
}

NetworkConnCtx &DiscoverUdpCli::getConnCtx()
{
    return *net_ctx;
}

DiscoveryTrigger::DiscoveryTrigger(struct timeval period, bool persist) : TriggerWithEvent(period, persist){};

void DiscoveryTrigger::exec()
{

    for (auto iter = conns.begin(); iter != conns.end(); iter++)
    {
        string msg = "hello";
        LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_HELLO);
        struct evbuffer *buf = evbuffer_new();
        pkt.write(buf);

        uint64_t data_len = evbuffer_get_length(buf);
        uint8_t *data = (uint8_t *)malloc(data_len);

        (*iter).second->getConnCtx().write(data, data_len);

        evbuffer_free(buf);
        free(data);
    }
}

bool DiscoveryTrigger::addNetAddr(NetAddr addr)
{
    DiscoverUdpCli *udpcli = conns[addr];
    if (udpcli != nullptr)
        return false;

    struct sockaddr_in peer = addr.getBeAddr();
    struct sockaddr_in *peerp = (struct sockaddr_in *)malloc(sizeof(sockaddr_in));
    memcpy(peerp, &peer, sizeof(sockaddr_in));
    udpcli = new DiscoverUdpCli(peerp);            // ne 何时释放
    NetworkConnCtx * nctx = NetworkLayerWithEvent::connectWithUdp(udpcli); // nctx 何时释放
    udpcli->setCtx(nctx);
    
    conns[addr] = udpcli;

    return true;
}

bool DiscoveryTrigger::delNetAddr(NetAddr addr)
{
    DiscoverUdpCli *udpcli = conns[addr];
    if (udpcli == nullptr)
        return false;
    else
        conns.erase(addr);

    return true;
}
