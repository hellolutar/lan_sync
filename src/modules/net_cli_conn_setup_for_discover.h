#ifndef __NET_CLI_CONN_SETUP_FOR_DISCOVER_H_
#define __NET_CLI_CONN_SETUP_FOR_DISCOVER_H_

#include "net/net_framework_impl_with_event.h"
#include "net/net_addr.h"
#include "proto/lan_share_protocol.h"
#include "abst_net_logic.h"
#include "net_trigger.h"

class NetCliConnSetupForDiscover : public AbstNetConnSetup
{
private:
    /* data */
public:
    NetCliConnSetupForDiscover(/* args */);
    ~NetCliConnSetupForDiscover();
    NetCliLogicContainer *setupConn(NetAddr peer, AbstNetLogic &logic) override;
    void exec(NetworkConnCtx &ctx) override;
};

#endif