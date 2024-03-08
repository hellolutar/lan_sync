#ifndef __NET_TRIGGER_H_
#define __NET_TRIGGER_H_

#include "timer/timer_trigger_with_event.h"
#include "proto/lan_share_protocol.h"
#include "net/network_layer_with_event.h"
#include "net/net_addr.h"
#include "logic.h"
#include "network_cli.h"

class NetCliConnBehavior
{
public:
    virtual ~NetCliConnBehavior(){};
    virtual void exec(NetworkConnCtx &ctx) = 0;
};

class NetCliConn : public NetCliConnBehavior
{
public:
    virtual ~NetCliConn();
    virtual NetworkCli *setupConn(struct sockaddr_in *peer, Logic &logic) = 0;
};

class NetTrigger : public TriggerWithEvent
{
private:
    std::map<NetAddr, NetworkCli *> conns;
    Logic &logic;
    NetCliConn &cliconn;

public:
    NetTrigger(struct timeval period, bool persist, Logic &logic, NetCliConn &cliconn);
    ~NetTrigger(){};

    void exec() override;
    bool addNetAddr(NetAddr addr);
    bool delNetAddr(NetAddr addr);
    // virtual NetTrigger &operator=(const NetTrigger &);
};

#endif