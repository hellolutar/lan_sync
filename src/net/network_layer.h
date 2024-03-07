#ifndef __NETWORK_LAYER_H_
#define __NETWORK_LAYER_H_

#include <cstdint>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class NetworkEndpoint;

class NetworkLayer
{
public:
    virtual ~NetworkLayer();
};

class NetworkConnCtx
{
protected:
    NetworkEndpoint *ne;
    std::vector<NetworkConnCtx *> *head;

public:
    NetworkConnCtx(std::vector<NetworkConnCtx *> *head, NetworkEndpoint *ne) : head(head), ne(ne){};
    virtual ~NetworkConnCtx();
    virtual uint64_t write(void *data, uint64_t data_len) = 0;
    virtual NetworkEndpoint *getNetworkEndpoint();
    virtual void destroy();
};

class NetworkEndpoint
{
protected:
    struct sockaddr_in *addr;
    int sock = 0;

public:
    NetworkEndpoint(struct sockaddr_in *addr) : addr(addr){};
    virtual ~NetworkEndpoint();

    virtual void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) = 0;
    virtual bool isExtraAllDataNow(void *data, uint64_t data_len) = 0;

    virtual struct sockaddr_in *getAddr();
    virtual void setSock(int sock);
};
#endif