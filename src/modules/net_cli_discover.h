#ifndef __NET_CLI_DISCOVER_
#define __NET_CLI_DISCOVER_

#include "timer/timer_trigger_with_event.h"
#include "proto/lan_share_protocol.h"
#include "net/network_layer_with_event.h"
#include "net/net_addr.h"
#include "logic.h"
#include "network_cli.h"
#include "net_trigger.h"

class NetCliConnDiscover : public NetCliConn
{
private:
    /* data */
public:
    NetCliConnDiscover(/* args */);
    ~NetCliConnDiscover();
    NetworkCli *setupConn(struct sockaddr_in *peer, Logic &logic);
    void exec(NetworkConnCtx &ctx) override;
};

#endif