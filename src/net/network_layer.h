#ifndef __NETWORK_LAYER_H_
#define __NETWORK_LAYER_H_

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <memory>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "net_addr.h"
/**
 * 网络抽象
 */
class NetAbility;

/**
 * 网络框架抽象
 */
class AbstNetFramework
{
public:
    virtual ~AbstNetFramework();
};

/**
 * 网络连接会话抽象，如TCP会话、UDP会话，用于发送消息
 */
class NetworkConnCtx
{
protected:
    NetAbility *ne = nullptr; // TODO 不大清楚这里的作用
    std::vector<std::shared_ptr<NetworkConnCtx>> *head;
    NetAddr peer;
    bool active = false;

public:
    NetworkConnCtx(){};
    NetworkConnCtx(std::vector<std::shared_ptr<NetworkConnCtx>> *head, NetAbility *ne, NetAddr peer) : head(head), ne(ne), peer(peer), active(true){};
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

/**
 * 网络抽象，用于接收消息
 */
class NetAbility
{
protected:
    NetAddr addr;
    int sock = 0;

public:
    NetAbility(NetAddr addr) : addr(addr){};
    virtual ~NetAbility();

    virtual void recv(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx) = 0;
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