#ifndef __LOGIC_H_
#define __LOGIC_H_

#include "net/network_layer.h"

class Logic
{
public:
    virtual ~Logic(){};
    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) = 0;
};

class LogicTcp : public Logic
{
public:
    virtual ~LogicTcp(){};
    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    virtual void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) = 0;
};

class LogicUdp : public Logic
{
public:
    virtual ~LogicUdp(){};
    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    virtual void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) = 0;
};

#endif
