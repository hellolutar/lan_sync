#include "net_trigger.h"

using namespace std;

NetTrigger::NetTrigger(struct timeval period, bool persist, AbstNetLogic &recv_logic, NetCliConnSetupTriggerBehavior *trigger_behavior)
    : TriggerWithEvent(period, persist), recv_logic(recv_logic), trigger_behavior(trigger_behavior){};

NetTrigger::~NetTrigger()
{
    delete trigger_behavior;
}

void NetTrigger::trigger()
{
    if (conns.size() == 0)
        LOG_WARN("NetTrigger::trigger() : can not trigger, beacuse the conns is empty!");

    for (auto iter = conns.begin(); iter != conns.end(); iter++)
    {
        trigger_behavior->exec((*iter).second->getConnCtx());
    }
}

bool NetTrigger::addConn(NetAddr addr)
{
    NetCliLogicContainer *netcli = conns[addr.str()];
    if (netcli != nullptr)
        return false;

    LOG_INFO("NetTrigger::addConn : {}", addr.str());

    netcli = trigger_behavior->setupConn(addr, recv_logic);

    conns[addr.str()] = netcli;

    return true;
}

bool NetTrigger::delNetAddr(NetAddr addr)
{
    NetCliLogicContainer *udpcli = conns[addr.str()];
    if (udpcli == nullptr)
        return false;
    else
        conns.erase(addr.str());

    return true;
}

// NetTrigger &NetTrigger::operator=(const NetTrigger &nt){
//     this->conns = nt.conns;
//     this->cliconn = nt.cliconn;
//     this->recv_logic = nt.recv_logic;
//     this->period = nt.period;
//     this->persist = nt.persist;

//     return *this;
// }