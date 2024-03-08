#ifndef __NET_FRAMEWORK_IMPL_WITH_EVENT_H_
#define __NET_FRAMEWORK_IMPL_WITH_EVENT_H_

#include <map>
#include <set>

#include <netinet/tcp.h>

#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

#include "network_layer.h"
#include "net_addr.h"
#include "utils/logger.h"

class NetworkConnCtxWithEvent;
class NetAbilityImplWithEvent;

class NetFrameworkImplWithEvent : public AbstNetFramework
{
private:
    static struct event_base *base;
    static std::vector<event *> events; // only persist event need to add
    static std::vector<NetworkConnCtx *> tcp_ctx;
    static std::vector<NetworkConnCtx *> udp_ctx;

    static void tcp_accept(evutil_socket_t listener, short event, void *ctx);
    static void init_check();

public:
    static void init(struct event_base *base);
    static void event_cb(struct bufferevent *bev, short events, void *data);
    static void write_cb(struct bufferevent *bev, void *data);
    static void read_cb(struct bufferevent *bev, void *data);
    static void udp_read_cb(evutil_socket_t fd, short events, void *ctx);

    static void addTcpServer(NetAbilityImplWithEvent *ne);
    static void addUdpServer(NetAbilityImplWithEvent *ne);
    static NetworkConnCtx *connectWithTcp(NetAbilityImplWithEvent *ne);
    static NetworkConnCtx *connectWithUdp(NetAbilityImplWithEvent *peer);
    static void run();
    static void shutdown();

    static void cleanup();
    static void free();
};

class NetworkConnCtxWithEvent : public NetworkConnCtx
{
private:
    struct bufferevent *bev;
    int peer_sock;

public:
    NetworkConnCtxWithEvent(std::vector<NetworkConnCtx *> *head, NetAbility *ne, struct bufferevent *bev, int peer_sock, NetAddr peer)
        : NetworkConnCtx(head, ne, peer), bev(bev), peer_sock(peer_sock){};

    ~NetworkConnCtxWithEvent();
    uint64_t write(void *data, uint64_t data_len) override;
};

class NetworkConnCtxWithEventForUDP : public NetworkConnCtx
{
private:
    int peer_sock;

public:
    NetworkConnCtxWithEventForUDP(std::vector<NetworkConnCtx *> *head, NetAbility *ne, int peer_sock, NetAddr peer_addr)
        : NetworkConnCtx(head, ne, peer_addr), peer_sock(peer_sock){};
    ~NetworkConnCtxWithEventForUDP();

    uint64_t write(void *data, uint64_t data_len) override;
};

class NetAbilityImplWithEvent : public NetAbility
{
protected:
    struct event *e = nullptr;

public:
    NetAbilityImplWithEvent(NetAddr addr) : NetAbility(addr){};
    ~NetAbilityImplWithEvent();
    void setEvent(struct event *e);
};

#endif