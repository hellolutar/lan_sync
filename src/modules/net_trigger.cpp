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
        LOG_DEBUG("NetTrigger::trigger() : can not trigger, beacuse the conns is empty!");

    for (auto iter = conns.begin(); iter != conns.end();)
    {
        NetCliLogicContainer *con = (*iter).second;
        auto meAddr = con->getAddr();
        auto peerAddr = con->getConnCtx().getPeer();
        try
        {
            NetworkConnCtx &ctx = (*iter).second->getConnCtx();
            trigger_behavior->exec(ctx);
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("NetTrigger::trigger() : {}", e.what());

            iter = conns.erase(iter);
            LOG_ERROR("NetTrigger::trigger() : delete connection: {} ", peerAddr.str().data());
            con->setCtx(nullptr);  // ctx release by itself, eg.~NetworkConnCtxWithEvent when ctx.write occur error
            delete con; // TODO there has a bug!
            continue;
        }
        iter++;
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