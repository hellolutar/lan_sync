#include "network_layer_tcp_with_event.h"

using namespace std;

struct event_base *NetworkLayerWithEvent::base;
std::map<struct bufferevent *, struct sockaddr_in *> NetworkLayerWithEvent::bev_peersock_table;
std::vector<event *> NetworkLayerWithEvent::events; // only persist event need to add
std::vector<NetworkContext *> NetworkLayerWithEvent::tcp_ctx;
std::vector<NetworkContext *> NetworkLayerWithEvent::udp_ctx;

void NetworkLayerWithEvent::event_cb(struct bufferevent *bev, short events, void *data)
{
}

void NetworkLayerWithEvent::write_cb(struct bufferevent *bev, void *data)
{
    LOG_INFO("[SYNC CLI] call tcp_writecb!");
}

void NetworkLayerWithEvent::read_cb(struct bufferevent *bev, void *arg)
{
    struct evbuffer *in = bufferevent_get_input(bev);

    uint32_t recvLen = evbuffer_get_length(in);
    if (recvLen == 0)
        return;

    // sync_server->handleTcpMsg(bev);
    uint64_t data_len = evbuffer_get_length(in);
    uint8_t *data = new uint8_t[data_len];

    NetworkContextWithEvent *ctx = (NetworkContextWithEvent *)arg;
    NetworkEndpoint *ne = ctx->getNetworkEndpoint();
    if (ne->isExtraAllDataNow((void *)data, data_len))
    {
        memset(data, 0, data_len);
        data_len = evbuffer_remove(in, data, data_len);

        ne->recv((void *)data, data_len, ctx);
    }
    delete[] data;
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

    NetworkContextWithEvent *nctx = new NetworkContextWithEvent(ne, bufferevent_get_output(bev));
    tcp_ctx.push_back(nctx);
    bufferevent_setcb(bev, read_cb, write_cb, nullptr, nctx);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

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

void NetworkLayerWithEvent::udp_read_cb(evutil_socket_t fd, short events, void *arg)
{
    struct sockaddr_in target_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char data[4096] = {0};

    uint32_t receive = recvfrom(fd, data, 4096, 0, (sockaddr *)&target_addr, &addrlen);
    if (receive <= 0)
    {
        LOG_WARN("[SYNC SER] cannot receive anything !");
        return;
    }
    NetworkContextWithEventForUDP *ctx = (NetworkContextWithEventForUDP *)arg;
    ctx->setPeerSockAddr(target_addr);
    NetworkEndpoint *ne = ctx->getNetworkEndpoint();
    ne->recv((void *)data, receive, ctx);
}

void NetworkLayerWithEvent::addUdpServer(NetworkEndpoint *ne)
{
    if (base == nullptr)
        base = event_base_new();

    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(udp_sock > 0);
    int optval = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    evutil_make_socket_nonblocking(udp_sock);

    assert(bind(udp_sock, (struct sockaddr *)ne->getAddr(), sizeof(struct sockaddr_in)) >= 0);

    LOG_INFO("UDP listen:");

    NetworkContextWithEventForUDP *ctx = new NetworkContextWithEventForUDP(ne, udp_sock);
    udp_ctx.push_back(ctx);

    struct event *read_e = event_new(base, udp_sock, EV_READ | EV_PERSIST, udp_read_cb, ctx);
    event_add(read_e, nullptr);
    events.push_back(read_e);
}

void NetworkLayerWithEvent::run()
{
    if (base == nullptr)
    {
        printf("need to addTcpServer/addUdpServer firstly!\n");
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

    for (size_t i = tcp_ctx.size() - 1; i >= 0; i--)
    {
        delete tcp_ctx[i];
    }

    for (size_t i = udp_ctx.size() - 1; i >= 0; i--)
    {
        delete udp_ctx[i];
    }
}

uint64_t NetworkContextWithEvent::write(void *data, uint64_t data_len)
{
    evbuffer_add(out, data, data_len);
    return 0;
}

NetworkContextWithEvent::~NetworkContextWithEvent()
{
}

NetworkContextWithEventForUDP::~NetworkContextWithEventForUDP()
{
}

void NetworkContextWithEventForUDP::setPeerSockAddr(struct sockaddr_in peer)
{
    this->peer = peer;
}

uint64_t NetworkContextWithEventForUDP::write(void *data, uint64_t data_len)
{
    return sendto(fd, data, data_len, 0, (struct sockaddr *)&peer, sizeof(struct sockaddr_in));
}