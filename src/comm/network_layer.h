#ifndef __NETWORK_LAYER_H_
#define __NETWORK_LAYER_H_

#include <cstdint>

class NetworkEndpoint;

class NetworkLayer
{
public:
    virtual ~NetworkLayer();
};

class NetworkContext
{
protected:
    NetworkEndpoint *ne;

public:
    NetworkContext(NetworkEndpoint *ne) : ne(ne){};
    virtual ~NetworkContext();
    virtual uint64_t write(void *data, uint64_t data_len) = 0;
    virtual NetworkEndpoint *getNetworkEndpoint();
};

class NetworkEndpoint
{
protected:
    struct sockaddr_in *addr;

public:
    NetworkEndpoint(struct sockaddr_in *addr) : addr(addr){};
    virtual ~NetworkEndpoint();

    virtual void recv(void *data, uint64_t data_len, NetworkContext *ctx) = 0;
    virtual bool isExtraAllDataNow(void *data, uint64_t data_len) = 0;

    virtual struct sockaddr_in *getAddr();
};

#endif