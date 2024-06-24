#ifndef __SYNC_TRIGGER_H_
#define __SYNC_TRIGGER_H_

#include <string>
#include <memory>

#include "constants/constants.h"
#include "modules/concrete_net_trigger.h"
#include "proto/lan_share_protocol.h"
#include "components/buf_base_on_event.h"

class SyncCliDiscoverLogic : public AbstNetConnTriggerBehavior, public ModConnAbility
{
public:
    SyncCliDiscoverLogic(/* args */);
    ~SyncCliDiscoverLogic(){};
    void exec(std::shared_ptr<NetworkConnCtx> &ctx) override;
    void mod_conn_recv(std::string from, std::string uri, void *data) override;
};

class DiscoveryTrigger : public UdpTrigger
{
private:
public:
    DiscoveryTrigger(struct timeval period, bool persist, AbstNetLogic &net_recv_logic, AbsModConnMediator *med);
    ~DiscoveryTrigger();
    virtual void mod_conn_recv(std::string from, std::string uri, void *data) override;
};

class SyncCliSyncLogic : public AbstNetConnTriggerBehavior, public ModConnAbility
{
public:
    SyncCliSyncLogic(/* args */){};
    ~SyncCliSyncLogic(){};
    virtual void exec(std::shared_ptr<NetworkConnCtx> &ctx) override;
    void reqTbIdx(std::shared_ptr<NetworkConnCtx> &ctx);
    void mod_conn_recv(std::string from, std::string uri, void *data) override;

};

class SyncReqTbIdxTrigger : public TcpTrigger
{
private:
    /* data */
public:
    SyncReqTbIdxTrigger(struct timeval period, bool persist, AbstNetLogic &net_recv_logic, AbsModConnMediator *med);
    ~SyncReqTbIdxTrigger();
    virtual void mod_conn_recv(std::string from, std::string uri, void *data) override;
};

#endif