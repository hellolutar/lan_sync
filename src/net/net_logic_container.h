#ifndef __NET_CLI_LOGIC_CONTAINER_H_
#define __NET_CLI_LOGIC_CONTAINER_H_

#include "net/net_framework_impl_with_event.h"
#include "net/net_addr.h"
#include "net/abst_net_logic.h"

class NetCliLogicContainer : public NetAbilityImplWithEvent
{
private:
    NetworkConnCtx *net_ctx;
    AbstNetLogic &recv_logic;

public:
    NetCliLogicContainer(NetAddr addr, AbstNetLogic &recv_logic) : NetAbilityImplWithEvent(addr), recv_logic(recv_logic){};
    virtual ~NetCliLogicContainer();

    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx);
    virtual uint64_t isExtraAllDataNow(void *data, uint64_t data_len);
    virtual void setCtx(NetworkConnCtx *ctx);

    NetworkConnCtx &getConnCtx();
};

class NetSrvLogicContainer : public NetAbilityImplWithEvent
{
private:
    AbstNetLogic &recv_logic;

public:
    NetSrvLogicContainer(NetAddr addr, AbstNetLogic &recv_logic) : NetAbilityImplWithEvent(addr), recv_logic(recv_logic){};
    ~NetSrvLogicContainer(){};

    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    virtual uint64_t isExtraAllDataNow(void *data, uint64_t data_len) override;
};

#endif