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
    for (auto iter = conns.begin(); iter != conns.end(); iter++)
    {
        trigger_behavior->exec((*iter).second->getConnCtx());
    }
}

bool NetTrigger::addNetAddr(NetAddr addr)
{
    NetCliLogicContainer *netcli = conns[addr];
    if (netcli != nullptr)
        return false;

    netcli = trigger_behavior->setupConn(addr, recv_logic);

    conns[addr] = netcli;

    return true;
}

bool NetTrigger::delNetAddr(NetAddr addr)
{
    NetCliLogicContainer *udpcli = conns[addr];
    if (udpcli == nullptr)
        return false;
    else
        conns.erase(addr);

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