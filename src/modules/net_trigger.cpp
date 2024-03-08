#include "net_trigger.h"

using namespace std;

AbstNetConnSetup::~AbstNetConnSetup()
{
}

NetTrigger::NetTrigger(struct timeval period, bool persist, Logic &logic, AbstNetConnSetup &cliconn)
    : TriggerWithEvent(period, persist), logic(logic), cliconn(cliconn){};

void NetTrigger::exec()
{
    for (auto iter = conns.begin(); iter != conns.end(); iter++)
    {
        cliconn.exec((*iter).second->getConnCtx());
    }
}

bool NetTrigger::addNetAddr(NetAddr addr)
{
    NetCliLogicContainer *netcli = conns[addr];
    if (netcli != nullptr)
        return false;

    netcli = cliconn.setupConn(addr, logic);

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
//     this->logic = nt.logic;
//     this->period = nt.period;
//     this->persist = nt.persist;

//     return *this;
// }