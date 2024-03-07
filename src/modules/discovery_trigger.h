#ifndef __DISCOVERY_TRIGGER_H_
#define __DISCOVERY_TRIGGER_H_

#include <sys/socket.h>

#include "timer/timer_trigger_with_event.h"
#include "proto/lan_share_protocol.h"
#include "net/network_layer_with_event.h"
#include "net/net_addr.h"

class DiscoverUdpCli : public NetworkEndpointWithEvent
{
private:
    NetworkConnCtx *net_ctx;

public:
    DiscoverUdpCli(struct sockaddr_in *addr) : NetworkEndpointWithEvent(addr){};
    ~DiscoverUdpCli();

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
    void setCtx(NetworkConnCtx * ctx);

    NetworkConnCtx& getConnCtx();
};

class DiscoveryTrigger : public TriggerWithEvent
{
private:
    int count = 0;
    std::map<NetAddr, DiscoverUdpCli *> conns;

public:
    DiscoveryTrigger(struct timeval period, bool persist);
    ~DiscoveryTrigger(){};

    void exec() override;
    bool addNetAddr(NetAddr addr);
    bool delNetAddr(NetAddr addr);
};

#endif