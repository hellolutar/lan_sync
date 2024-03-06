#ifndef __NETWORK_LAYER_H_
#define __NETWORK_LAYER_H_

#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
    int sock;

public:
    NetworkEndpoint(struct sockaddr_in *addr) : addr(addr){};
    virtual ~NetworkEndpoint();

    virtual void recv(void *data, uint64_t data_len, NetworkContext *ctx) = 0;
    virtual bool isExtraAllDataNow(void *data, uint64_t data_len) = 0;

    virtual struct sockaddr_in *getAddr();
    virtual void setSock(int sock);
};


class NetworkOutputStream
{
public:
    virtual ~NetworkOutputStream() = 0;

    virtual uint64_t write(void *data, uint64_t data_len) = 0;
};

class NetworkOutputStreamForUDP : public NetworkOutputStream
{
protected:
    int sock;
    struct sockaddr_in peer;

public:
    NetworkOutputStreamForUDP(int sock, struct sockaddr_in peer) : sock(sock), peer(peer){};
    virtual ~NetworkOutputStreamForUDP();

    virtual uint64_t write(void *data, uint64_t data_len) override;
};

#endif