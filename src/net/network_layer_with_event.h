#ifndef __NETWORK_LAYER_TCP_WITH_EVENT_H_
#define __NETWORK_LAYER_TCP_WITH_EVENT_H_

#include <map>
#include <set>

#include <netinet/tcp.h>

#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

#include "network_layer.h"
#include "utils/logger.h"

class NetworkConnCtxWithEvent;
class NetworkEndpointWithEvent;

class NetworkLayerWithEvent : public NetworkLayer
{
private:
    static struct event_base *base;
    static std::vector<event *> events; // only persist event need to add
    static std::vector<NetworkConnCtx *> tcp_ctx;
    static std::vector<NetworkConnCtx *> udp_ctx;

    static void tcp_accept(evutil_socket_t listener, short event, void *ctx);

public:
    static void event_cb(struct bufferevent *bev, short events, void *data);
    static void write_cb(struct bufferevent *bev, void *data);
    static void read_cb(struct bufferevent *bev, void *data);
    static void udp_read_cb(evutil_socket_t fd, short events, void *ctx);

    static void addTcpServer(NetworkEndpointWithEvent *ne);
    static void addUdpServer(NetworkEndpointWithEvent *ne);
    static NetworkConnCtx *connectWithTcp(NetworkEndpointWithEvent *ne);
    static NetworkConnCtx *connectWithUdp(NetworkEndpointWithEvent *peer);
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
    struct sockaddr_in peer_addr;

public:
    NetworkConnCtxWithEvent(std::vector<NetworkConnCtx *> *head, NetworkEndpoint *ne, struct bufferevent *bev, int peer_sock, struct sockaddr_in peer_addr)
        : NetworkConnCtx(head, ne), bev(bev), peer_sock(peer_sock), peer_addr(peer_addr){};

    ~NetworkConnCtxWithEvent();
    uint64_t write(void *data, uint64_t data_len) override;
};

class NetworkConnCtxWithEventForUDP : public NetworkConnCtx
{
private:
    int peer_sock;
    struct sockaddr_in peer_addr;

public:
    NetworkConnCtxWithEventForUDP(std::vector<NetworkConnCtx *> *head, NetworkEndpoint *ne, int peer_sock, struct sockaddr_in peer_addr)
        : NetworkConnCtx(head, ne), peer_sock(peer_sock), peer_addr(peer_addr){};
    ~NetworkConnCtxWithEventForUDP();

    uint64_t write(void *data, uint64_t data_len) override;
    void setPeerSockAddr(struct sockaddr_in peer_addr);
};

class NetworkEndpointWithEvent : public NetworkEndpoint
{
protected:
    struct event *e = nullptr;

public:
    NetworkEndpointWithEvent(struct sockaddr_in *addr) : NetworkEndpoint(addr){};
    ~NetworkEndpointWithEvent();
    void setEvent(struct event *e);
};

#endif