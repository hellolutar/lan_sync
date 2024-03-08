#ifndef __NETWORK_LAYER_H_
#define __NETWORK_LAYER_H_

#include <cstdint>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net_addr.h"

class NetAbility;

class AbstNetFramework
{
public:
    virtual ~AbstNetFramework();
};

class NetworkConnCtx
{
protected:
    NetAbility *ne;
    std::vector<NetworkConnCtx *> *head;
    NetAddr peer;

public:
    NetworkConnCtx(std::vector<NetworkConnCtx *> *head, NetAbility *ne, NetAddr peer) : head(head), ne(ne), peer(peer){};
    virtual ~NetworkConnCtx();
    virtual uint64_t write(void *data, uint64_t data_len) = 0;
    virtual NetAbility *getNetworkEndpoint();
    virtual void destroy();
    virtual NetAddr &getPeer();
    virtual void setNetAddr(NetAddr peer);
};

class NetAbility
{
protected:
    NetAddr addr;
    int sock = 0;

public:
    NetAbility(NetAddr addr) : addr(addr){};
    virtual ~NetAbility();

    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) = 0;
    virtual bool isExtraAllDataNow(void *data, uint64_t data_len) = 0;

    virtual NetAddr &getAddr();
    virtual void setSock(int sock);
};
#endif