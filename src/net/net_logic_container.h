#ifndef __NET_CLI_LOGIC_CONTAINER_H_
#define __NET_CLI_LOGIC_CONTAINER_H_

#include "net/net_framework_impl_with_event.h"
#include "net/net_addr.h"
#include "net/abst_net_logic.h"

class NetCliLogicContainer : public NetAbilityImplWithEvent
{
private:
    std::shared_ptr<NetworkConnCtx> net_ctx;
    AbstNetLogic &recv_logic;

public:
    NetCliLogicContainer(NetAddr addr, AbstNetLogic &recv_logic) : NetAbilityImplWithEvent(addr), recv_logic(recv_logic){};
    virtual ~NetCliLogicContainer();

    virtual void recv(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx);
    virtual uint64_t isExtraAllDataNow(void *data, uint64_t data_len);
    virtual void setCtx(std::shared_ptr<NetworkConnCtx> ctx);

    std::shared_ptr<NetworkConnCtx>& getConnCtx();
};

class NetSrvLogicContainer : public NetAbilityImplWithEvent
{
private:
    AbstNetLogic &recv_logic;

public:
    NetSrvLogicContainer(NetAddr addr, AbstNetLogic &recv_logic) : NetAbilityImplWithEvent(addr), recv_logic(recv_logic){};
    ~NetSrvLogicContainer(){};

    virtual void recv(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx) override;
    virtual uint64_t isExtraAllDataNow(void *data, uint64_t data_len) override;
};

#endif