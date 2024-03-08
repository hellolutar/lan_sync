#ifndef __ABST_NET_CONN_SETUP_H_
#define __ABST_NET_CONN_SETUP_H_

#include "net_cli_logic_container.h"

class NetCliConnBehavior
{
public:
    virtual ~NetCliConnBehavior(){};
    virtual void exec(NetworkConnCtx &ctx) = 0;
};

class AbstNetConnSetup : public NetCliConnBehavior
{
public:
    virtual ~AbstNetConnSetup(){};
    virtual NetCliLogicContainer *setupConn(NetAddr peer, AbstNetLogic &logic) = 0;
};

#endif