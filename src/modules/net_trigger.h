#ifndef __NET_TRIGGER_H_
#define __NET_TRIGGER_H_

#include "timer/timer_trigger_with_event.h"
#include "net/net_addr.h"
#include "modules/abst_net_conn_setup.h"
#include "modules/concrete_net_setup.h"
#include "modules/conn/mod_conn.h"
#include "utils/str_utils.h"

class NetTrigger : public TriggerWithEvent, public ModConnAbility
{
protected:
    std::map<std::string, NetCliLogicContainer *> conns;
    AbstNetLogic &recv_logic;
    NetCliConnSetupTriggerBehavior *trigger_behavior;
public:
    NetTrigger(struct timeval period, bool persist, AbstNetLogic &recv_logic, NetCliConnSetupTriggerBehavior *cliconn);
    virtual ~NetTrigger();
    
    virtual void trigger() override;
    virtual bool addConn(NetAddr addr);
    virtual bool delNetAddr(NetAddr addr);
};

#endif
