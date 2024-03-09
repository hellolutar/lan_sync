#ifndef __NET_TRIGGER_H_
#define __NET_TRIGGER_H_

#include "timer/timer_trigger_with_event.h"
#include "net/net_addr.h"
#include "modules/abst_net_conn_setup.h"
#include "modules/concrete_net_setup.h"

class NetTrigger : public TriggerWithEvent
{
protected:
    std::map<NetAddr, NetCliLogicContainer *> conns;
    AbstNetLogic &recv_logic;
    NetCliConnSetupTriggerBehavior *trigger_behavior;

public:
    NetTrigger(struct timeval period, bool persist, AbstNetLogic &recv_logic, NetCliConnSetupTriggerBehavior *cliconn);
    ~NetTrigger();

    void trigger() override;
    bool addNetAddr(NetAddr addr);
    bool delNetAddr(NetAddr addr);
    // virtual NetTrigger &operator=(const NetTrigger &);
};

#endif
