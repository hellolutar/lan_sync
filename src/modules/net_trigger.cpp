#include "net_trigger.h"

using namespace std;

NetCliConn::~NetCliConn()
{
}

NetTrigger::NetTrigger(struct timeval period, bool persist, Logic &logic, NetCliConn &cliconn)
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
    NetworkCli *netcli = conns[addr];
    if (netcli != nullptr)
        return false;

    struct sockaddr_in peer = addr.getBeAddr();
    struct sockaddr_in *peerp = (struct sockaddr_in *)malloc(sizeof(sockaddr_in));
    memcpy(peerp, &peer, sizeof(sockaddr_in));

    netcli = cliconn.setupConn(peerp, logic);

    conns[addr] = netcli;

    return true;
}

bool NetTrigger::delNetAddr(NetAddr addr)
{
    NetworkCli *udpcli = conns[addr];
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