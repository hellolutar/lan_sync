#ifndef __NETWORK_CLI_H_
#define __NETWORK_CLI_H_

#include "timer/timer_trigger_with_event.h"
#include "proto/lan_share_protocol.h"
#include "net/network_layer_with_event.h"
#include "net/net_addr.h"
#include "logic.h"

class NetCliLogicContainer : public NetAbilityImplWithEvent
{
private:
    NetworkConnCtx *net_ctx;
    Logic &logic;

public:
    NetCliLogicContainer(NetAddr addr, Logic &logic) : NetAbilityImplWithEvent(addr), logic(logic){};
    virtual ~NetCliLogicContainer(){};

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
    void setCtx(NetworkConnCtx *ctx);

    NetworkConnCtx &getConnCtx();
};

#endif