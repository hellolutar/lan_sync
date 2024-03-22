#ifndef __NETWORK_LAYER_H_
#define __NETWORK_LAYER_H_

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

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
    bool active;

public:
    NetworkConnCtx(std::vector<NetworkConnCtx *> *head, NetAbility *ne, NetAddr peer) : head(head), ne(ne), peer(peer), active(true){};
    /**
     *  resource release by the caller with trycatch
     */
    virtual ~NetworkConnCtx();
    virtual uint64_t write(void *data, uint64_t data_len) = 0;
    virtual NetAbility *getNetworkEndpoint();
    virtual NetAddr &getPeer();
    virtual void setNetAddr(NetAddr peer);
    virtual bool isActive();
    virtual void setActive(bool status);
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
    virtual uint64_t isExtraAllDataNow(void *data, uint64_t data_len) = 0;

    virtual NetAddr &getAddr();
    virtual void setSock(int sock);
};

struct NetworkConnCtxException : public std::exception
{
    const char *what() const throw()
    {
        return "NetworkConnCtxException";
    }
};
#endif