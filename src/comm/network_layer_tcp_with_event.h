#ifndef __NETWORK_LAYER_TCP_WITH_EVENT_H_
#define __NETWORK_LAYER_TCP_WITH_EVENT_H_

#include <map>
#include <set>

#include <netinet/tcp.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

#include "network_layer.h"
#include "utils/logger.h"

class NetworkLayerWithEvent : public NetworkLayer
{
private:
    static struct event_base *base;
    static std::map<struct bufferevent *, struct sockaddr_in *> bev_peersock_table;
    static std::vector<event *> events; // only persist event need to add
    static std::vector<NetworkContext *> tcp_ctx;
    static std::vector<NetworkContext *> udp_ctx;

    static void tcp_accept(evutil_socket_t listener, short event, void *ctx);

public:
    static void event_cb(struct bufferevent *bev, short events, void *data);
    static void write_cb(struct bufferevent *bev, void *data);
    static void read_cb(struct bufferevent *bev, void *data);
    static void udp_read_cb(evutil_socket_t fd, short events, void *ctx);

    static void addTcpServer(NetworkEndpoint *ne);
    static void addUdpServer(NetworkEndpoint *ne);
    static void run();

    static void free();
};

class NetworkContextWithEvent : public NetworkContext
{
private:
    struct evbuffer *out; // release by libevent

public:
    NetworkContextWithEvent(NetworkEndpoint *ne, struct evbuffer *out) : NetworkContext(ne), out(out){};
    ~NetworkContextWithEvent();
    uint64_t write(void *data, uint64_t data_len) override;
};

class NetworkContextWithEventForUDP : public NetworkContext
{
private:
    int fd;
    struct sockaddr_in peer;

public:
    NetworkContextWithEventForUDP(NetworkEndpoint *ne, int sock) : NetworkContext(ne), fd(sock){};
    ~NetworkContextWithEventForUDP();
    void setPeerSockAddr(struct sockaddr_in peer);
    uint64_t write(void *data, uint64_t data_len) override;
};

#endif