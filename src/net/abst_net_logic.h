#ifndef __ABST_NET_LOGIC_H_
#define __ABST_NET_LOGIC_H_

#include "net/network_layer.h"

class AbstNetLogic
{
public:
    virtual ~AbstNetLogic(){};
    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) = 0;
    virtual void isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len) = 0;
};

class LogicTcp : public AbstNetLogic
{
public:
    virtual ~LogicTcp(){};
    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    virtual void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) = 0;
};

class LogicUdp : public AbstNetLogic
{
public:
    virtual ~LogicUdp(){};
    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    virtual void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) = 0;
};

#endif
