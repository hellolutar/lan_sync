#ifndef __NET_CLI_CONN_SETUP_FOR_REQ_TB_IDX_H_
#define __NET_CLI_CONN_SETUP_FOR_REQ_TB_IDX_H_

#include "net/net_framework_impl_with_event.h"
#include "net/net_addr.h"
#include "abst_net_logic.h"
#include "net_trigger.h"

class NetCliConnSetupForReqTbIdx : public AbstNetConnSetup
{
private:
    /* data */
public:
    NetCliConnSetupForReqTbIdx(/* args */);
    ~NetCliConnSetupForReqTbIdx();
    NetCliLogicContainer *setupConn(NetAddr peer, AbstNetLogic &logic) override;
    void exec(NetworkConnCtx &ctx) override;
};

#endif