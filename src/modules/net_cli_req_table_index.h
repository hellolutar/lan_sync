#ifndef __NET_CLI_REQ_TABLE_INDEX_H_
#define __NET_CLI_REQ_TABLE_INDEX_H_

#include "timer/timer_trigger_with_event.h"
#include "proto/lan_share_protocol.h"
#include "net/network_layer_with_event.h"
#include "net/net_addr.h"
#include "logic.h"
#include "network_cli.h"
#include "net_trigger.h"

class NetCliConnReqTableIndex : public AbstNetConnSetup
{
private:
    /* data */
public:
    NetCliConnReqTableIndex(/* args */);
    ~NetCliConnReqTableIndex();
    NetCliLogicContainer *setupConn(NetAddr peer, Logic &logic) override;
    void exec(NetworkConnCtx &ctx) override;
};

#endif