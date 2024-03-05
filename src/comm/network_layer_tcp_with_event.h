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
    static std::map<struct bufferevent *, NetworkEndpoint *> bev_ept_table;
    static std::map<struct bufferevent *, struct sockaddr_in *> bev_peersock_table;
    static std::vector<event *> events; // only persist event need to add

    static void tcp_accept(evutil_socket_t listener, short event, void *ctx);

public:
    static void event_cb(struct bufferevent *bev, short events, void *data);
    static void write_cb(struct bufferevent *bev, void *data);
    static void read_cb(struct bufferevent *bev, void *data);

    static void addTcpServer(NetworkEndpoint *ne);
    static void run();
    
    static void send(NetworkEndpoint *from, void *data, uint64_t data_len);

    static void free();
};

#endif