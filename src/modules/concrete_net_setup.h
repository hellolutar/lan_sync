#ifndef __CONCRETE_NET_SETUP_H_
#define __CONCRETE_NET_SETUP_H_

#include "modules/abst_net_conn_setup.h"

class AbstNetConnTriggerBehavior
{
public:
    virtual ~AbstNetConnTriggerBehavior(){};
    virtual void exec(std::shared_ptr<NetworkConnCtx> &ctx) = 0;
};

class NetCliConnSetupTriggerBehavior : public AbstNetConnSetup, public AbstNetConnTriggerBehavior
{
protected:
    AbstNetConnTriggerBehavior &trigger_logic;

public:
    NetCliConnSetupTriggerBehavior(AbstNetConnTriggerBehavior &trigger_logic) : trigger_logic(trigger_logic){};
    ~NetCliConnSetupTriggerBehavior(){};
};

class TcpCliConnSetUp : public NetCliConnSetupTriggerBehavior
{

public:
    TcpCliConnSetUp(AbstNetConnTriggerBehavior &trigger_logic) : NetCliConnSetupTriggerBehavior(trigger_logic){};
    ~TcpCliConnSetUp(){};
    NetCliLogicContainer *setupConn(NetAddr peer, AbstNetLogic &recv_logic) override;
    void exec(std::shared_ptr<NetworkConnCtx> &ctx) override;
};

class UdpCliConnSetUp : public NetCliConnSetupTriggerBehavior
{

public:
    UdpCliConnSetUp(AbstNetConnTriggerBehavior &trigger_logic) : NetCliConnSetupTriggerBehavior(trigger_logic){};
    ~UdpCliConnSetUp(){};
    NetCliLogicContainer *setupConn(NetAddr peer, AbstNetLogic &recv_logic) override;
    void exec(std::shared_ptr<NetworkConnCtx> &ctx) override;
};

#endif