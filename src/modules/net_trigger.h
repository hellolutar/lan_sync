#ifndef __NET_TRIGGER_H_
#define __NET_TRIGGER_H_

#include "timer/timer_trigger_with_event.h"
#include "net/net_addr.h"
#include "abst_net_conn_setup.h"


class NetTrigger : public TriggerWithEvent
{
private:
    std::map<NetAddr, NetCliLogicContainer *> conns;
    AbstNetLogic &logic;
    AbstNetConnSetup &cliconn;

public:
    NetTrigger(struct timeval period, bool persist, AbstNetLogic &logic, AbstNetConnSetup &cliconn);
    ~NetTrigger(){};

    void exec() override;
    bool addNetAddr(NetAddr addr);
    bool delNetAddr(NetAddr addr);
    // virtual NetTrigger &operator=(const NetTrigger &);
};

#endif