#include "network_layer_tcp_with_event.h"

using namespace std;

struct event_base *NetworkLayerWithEvent::base;
std::map<struct bufferevent *, NetworkEndpoint *> NetworkLayerWithEvent::bev_ept_table;
std::map<struct bufferevent *, struct sockaddr_in *> NetworkLayerWithEvent::bev_peersock_table;
std::vector<event *> NetworkLayerWithEvent::events; // only persist event need to add

void NetworkLayerWithEvent::event_cb(struct bufferevent *bev, short events, void *data)
{
}

void NetworkLayerWithEvent::write_cb(struct bufferevent *bev, void *data)
{
    LOG_INFO("[SYNC CLI] call tcp_writecb!");
}

void NetworkLayerWithEvent::read_cb(struct bufferevent *bev, void *data)
{
    struct evbuffer *in = bufferevent_get_input(bev);

    uint32_t recvLen = evbuffer_get_length(in);
    if (recvLen == 0)
        return;

    // todo调用注册表格注册
    // sync_server->handleTcpMsg(bev);
    NetworkEndpoint *ne = bev_ept_table[bev];
    if (ne == nullptr)
        LOG_WARN("NetworkLayerWithEvent::read_cb  query NetworkEndpoint from bev_ept_table: is nullptr");
    else
    {
        uint64_t data_len = evbuffer_get_length(in);
        uint8_t *data = new uint8_t[data_len];
        if (ne->isExtraAllDataNow((void *)data, data_len))
        {
            memset(data, 0, data_len);
            data_len = evbuffer_remove(in, data, data_len);
            ne->recv((void *)data, data_len);
        }
        delete[] data;
    }
}

// todo free event

void NetworkLayerWithEvent::send(NetworkEndpoint *from, void *data, uint64_t data_len)
{
    for (auto iter = bev_ept_table.begin(); iter != bev_ept_table.end(); iter++)
    {
        if (iter->second == from)
        {
            struct bufferevent *bev = iter->first;
            struct evbuffer *out = bufferevent_get_output(bev);
            evbuffer_add(out, data, data_len);
            break;
        }
    }
    LOG_WARN("NetworkLayerWithEvent::send");
}

void NetworkLayerWithEvent::tcp_accept(evutil_socket_t listener, short event, void *ctx)
{
    NetworkEndpoint *ne = (NetworkEndpoint *)ctx;

    struct sockaddr_in *peer = new sockaddr_in();
    socklen_t slen = sizeof(struct sockaddr_in);

    int fd = accept(listener, (struct sockaddr *)peer, &slen);
    assert(fd > 0 && fd < FD_SETSIZE);

    struct bufferevent *bev;
    evutil_make_socket_nonblocking(fd);
    evutil_make_listen_socket_reuseable_port(fd);

    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    size_t hw = 0;
    size_t lw = 0;
    bufferevent_getwatermark(bev, EV_WRITE, &lw, &hw);
    bufferevent_setwatermark(bev, EV_WRITE, 0, hw);
    bufferevent_setcb(bev, read_cb, write_cb, nullptr, base);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    bev_ept_table[bev] = ne;
    bev_peersock_table[bev] = peer;
}

void NetworkLayerWithEvent::addTcpServer(NetworkEndpoint *ne)
{
    if (base == nullptr)
        base = event_base_new();

    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(tcp_sock > 0);
    evutil_make_socket_nonblocking(tcp_sock);
    int optval = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    setsockopt(tcp_sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int));

    assert(bind(tcp_sock, (struct sockaddr *)ne->getAddr(), sizeof(struct sockaddr_in)) == 0);

    int res = listen(tcp_sock, 100);
    if (res == -1)
    {
        LOG_ERROR("[SYNC SER] : {} ", strerror(errno));
        exit(-1);
    }
    // LOG_INFO("[SYNC SER] TCP listen : {}", DISCOVER_SERVER_TCP_PORT);

    struct event *accept_event_persist = event_new(base, tcp_sock, EV_READ | EV_PERSIST, tcp_accept, (void *)ne);
    event_add(accept_event_persist, nullptr);

    events.push_back(accept_event_persist);
}

void NetworkLayerWithEvent::udp_read_cb(evutil_socket_t fd, short events, void *ctx)
{
    NetworkEndpoint *ne = (NetworkEndpoint *)ctx;

    struct sockaddr_in target_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char data[4096] = {0};

    uint32_t receive = recvfrom(fd, data, 4096, 0, (sockaddr *)&target_addr, &addrlen);
    if (receive <= 0)
    {
        LOG_WARN("[SYNC SER] cannot receive anything !");
        return;
    }
    ne->recv((void *)data, receive);

}

void NetworkLayerWithEvent::addUdpServer(NetworkEndpoint *ne)
{
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(udp_sock > 0);
    int optval = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    evutil_make_socket_nonblocking(udp_sock);

    assert(bind(udp_sock, (struct sockaddr *)ne->getAddr(), sizeof(struct sockaddr_in)) >= 0);

    LOG_INFO("UDP listen:");

    struct event *read_e = event_new(base, udp_sock, EV_READ | EV_PERSIST, udp_read_cb, base);
    event_add(read_e, nullptr);
    events.push_back(read_e);

    // todo table for sockaddr

}

void NetworkLayerWithEvent::run()
{
    if (base == nullptr)
    {
        printf("need to addTcpServer firstly!\n");
        return;
    }

    event_base_dispatch(base);
}

void NetworkLayerWithEvent::free()
{
    event_base_free(base);

    for (size_t i = events.size() - 1; i >= 0; i--)
    {
        struct event *e = events[i];
        event_free(e);
    }

    set<struct bufferevent *> bevs;
    set<struct sockaddr_in *> socks;

    for (auto iter = bev_ept_table.begin(); iter != bev_ept_table.end();)
    {
        bevs.insert(iter->first);
        delete iter->second;
        iter = bev_ept_table.erase(iter);
    }

    for (auto iter = bev_peersock_table.begin(); iter != bev_peersock_table.end();)
    {
        bevs.insert(iter->first);
        socks.insert(iter->second);
        iter = bev_peersock_table.erase(iter);
    }

    for (auto iter : bevs)
    {
        bevs.erase(iter);
    }

    for (auto iter : socks)
    {
        socks.erase(iter);
    }
}
