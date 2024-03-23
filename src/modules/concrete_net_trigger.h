#ifndef __CONCRETE_NET_TRIGGER_H_
#define __CONCRETE_NET_TRIGGER_H_

#include "modules/net_trigger.h"

class UdpTrigger : public NetTrigger
{
private:
    UdpTrigger(struct timeval period, bool persist, AbstNetLogic &recv_logic, NetCliConnSetupTriggerBehavior *cliconn);

public:
    UdpTrigger(struct timeval period, bool persist, AbstNetLogic &net_recv_logic, AbstNetConnTriggerBehavior &trigger_logic)
        : NetTrigger(period, persist, net_recv_logic, new UdpCliConnSetUp(trigger_logic)) {}
    virtual ~UdpTrigger(){};
};

class TcpTrigger : public NetTrigger
{
private:
    TcpTrigger(struct timeval period, bool persist, AbstNetLogic &recv_logic, NetCliConnSetupTriggerBehavior *cliconn);

public:
    TcpTrigger(struct timeval period, bool persist, AbstNetLogic &net_recv_logic, AbstNetConnTriggerBehavior &trigger_logic)
        : NetTrigger(period, persist, net_recv_logic, new TcpCliConnSetUp(trigger_logic)) {}
    virtual ~TcpTrigger(){};
};

#endif