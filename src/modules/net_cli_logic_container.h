#ifndef __NET_CLI_LOGIC_CONTAINER_H_
#define __NET_CLI_LOGIC_CONTAINER_H_

#include "net/net_framework_impl_with_event.h"
#include "net/net_addr.h"
#include "abst_net_logic.h"

class NetCliLogicContainer : public NetAbilityImplWithEvent
{
private:
    NetworkConnCtx *net_ctx;
    AbstNetLogic &logic;

public:
    NetCliLogicContainer(NetAddr addr, AbstNetLogic &logic) : NetAbilityImplWithEvent(addr), logic(logic){};
    virtual ~NetCliLogicContainer(){};

    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx);
    virtual bool isExtraAllDataNow(void *data, uint64_t data_len);
    virtual void setCtx(NetworkConnCtx *ctx);

    NetworkConnCtx &getConnCtx();
};

#endif