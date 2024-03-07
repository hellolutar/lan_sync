#include "network_layer_with_event.h"

using namespace std;

struct event_base *NetworkLayerWithEvent::base;
std::vector<event *> NetworkLayerWithEvent::events; // only persist event need to add
std::vector<NetworkConnCtx *> NetworkLayerWithEvent::tcp_ctx;
std::vector<NetworkConnCtx *> NetworkLayerWithEvent::udp_ctx;

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

    uint64_t data_len = evbuffer_get_length(in);
    uint8_t *data = new uint8_t[data_len];
    evbuffer_copyout(in, data, data_len);

    NetworkConnCtxWithEvent *ctx = (NetworkConnCtxWithEvent *)arg;
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

    struct sockaddr_in peer;
    socklen_t slen = sizeof(struct sockaddr_in);

    int peer_sock = accept(listener, (struct sockaddr *)&peer, &slen);
    assert(peer_sock > 0 && peer_sock < FD_SETSIZE);

    evutil_make_socket_nonblocking(peer_sock);
    evutil_make_listen_socket_reuseable_port(peer_sock);

    struct bufferevent *bev = bufferevent_socket_new(base, peer_sock, BEV_OPT_CLOSE_ON_FREE);
    size_t hw = 0;
    size_t lw = 0;
    bufferevent_getwatermark(bev, EV_WRITE, &lw, &hw);
    bufferevent_setwatermark(bev, EV_WRITE, 0, hw);

    NetworkConnCtxWithEvent *nctx = new NetworkConnCtxWithEvent(&tcp_ctx, ne, bev, peer_sock, peer);
    tcp_ctx.push_back(nctx);
    bufferevent_setcb(bev, read_cb, write_cb, nullptr, nctx);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void NetworkLayerWithEvent::addTcpServer(NetworkEndpointWithEvent *ne)
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
        delete ne;
        return;
    }
    ne->setSock(tcp_sock);

    LOG_INFO("TCP listen: {}", ntohs(ne->getAddr()->sin_port));

    struct event *accept_event_persist = event_new(base, tcp_sock, EV_READ | EV_PERSIST, tcp_accept, (void *)ne);
    event_add(accept_event_persist, nullptr);

    ne->setEvent(accept_event_persist);
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
    NetworkConnCtxWithEventForUDP *nctx = (NetworkConnCtxWithEventForUDP *)arg;
    nctx->setPeerSockAddr(target_addr);
    NetworkEndpoint *ne = nctx->getNetworkEndpoint();
    ne->recv((void *)data, receive, nctx);
}

void NetworkLayerWithEvent::addUdpServer(NetworkEndpointWithEvent *ne)
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
    ne->setSock(udp_sock);

    LOG_INFO("UDP listen: {}", ntohs(ne->getAddr()->sin_port));

    struct sockaddr_in addr_will_be_udpate_follow;
    NetworkConnCtxWithEventForUDP *nctx = new NetworkConnCtxWithEventForUDP(&udp_ctx, ne, udp_sock, addr_will_be_udpate_follow); // udp_sock close many time: 1.NetworkEndpointWithEvent; 2.NetworkConnCtx
    udp_ctx.push_back(nctx);

    struct event *read_e = event_new(base, udp_sock, EV_READ | EV_PERSIST, udp_read_cb, nctx);
    event_add(read_e, nullptr);
    ne->setEvent(read_e);
}

NetworkConnCtx *NetworkLayerWithEvent::connectWithTcp(NetworkEndpointWithEvent *peer_ne)
{
    if (base == nullptr)
        base = event_base_new();

    struct sockaddr_in *target_addr = peer_ne->getAddr();

    auto tmpaddr = target_addr->sin_addr;
    tmpaddr.s_addr = ntohl(tmpaddr.s_addr);
    string ip(inet_ntoa(tmpaddr));
    string port = to_string(ntohs(target_addr->sin_port));
    LOG_INFO("TCP connect : {}:{}", ip, port);

    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int peer_sock = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(peer_sock, (struct sockaddr *)peer_ne->getAddr(), sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        LOG_ERROR("TCP connect : {}:{}  REASON:{} ", ip, port, strerror(errno));
        delete peer_ne;
        return nullptr;
    }

    struct bufferevent *bev = bufferevent_socket_new(base, peer_sock, BEV_OPT_CLOSE_ON_FREE);

    NetworkConnCtxWithEvent *nctx = new NetworkConnCtxWithEvent(&tcp_ctx, peer_ne, bev, peer_sock, *(peer_ne->getAddr())); //
    tcp_ctx.push_back(nctx);
    bufferevent_setcb(bev, read_cb, write_cb, nullptr, nctx);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    return nctx; // todo determine delete
}

NetworkConnCtx *NetworkLayerWithEvent::connectWithUdp(NetworkEndpointWithEvent *peer_ne)
{
    if (base == nullptr)
        base = event_base_new();

    int peer_sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(peer_sock > 0);

    int optval = 1; // 这个值一定要设置，否则可能导致sendto()失败
    setsockopt(peer_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(peer_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(peer_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    evutil_make_socket_nonblocking(peer_sock);

    struct sockaddr_in *target_addr = peer_ne->getAddr(); // target_addr free by peer

    auto tmpaddr = target_addr->sin_addr;
    tmpaddr.s_addr = ntohl(tmpaddr.s_addr);
    string ip(inet_ntoa(tmpaddr));
    string port = to_string(ntohs(target_addr->sin_port));
    LOG_INFO("UDP connect : {}:{}", ip, port);

    NetworkConnCtxWithEventForUDP *nctx = new NetworkConnCtxWithEventForUDP(&udp_ctx, peer_ne, peer_sock, *(peer_ne->getAddr()));
    udp_ctx.push_back(nctx);

    struct event *read_e = event_new(base, peer_sock, EV_READ | EV_PERSIST, udp_read_cb, nctx);
    event_add(read_e, nullptr);
    peer_ne->setEvent(read_e);

    return nctx;
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

void NetworkLayerWithEvent::shutdown()
{
    event_base_loopbreak(base);
    cleanup();
}

void NetworkLayerWithEvent::cleanup()
{
    for (int i = events.size() - 1; i >= 0; i--)
    {
        struct event *e = events[i];
        event_free(e);
        events.pop_back();
    }

    for (int i = tcp_ctx.size() - 1; i >= 0; i--)
    {
        delete tcp_ctx[i];
        tcp_ctx.pop_back();
    }

    for (int i = udp_ctx.size() - 1; i >= 0; i--)
    {
        delete udp_ctx[i];
        udp_ctx.pop_back();
    }
}

void NetworkLayerWithEvent::free()
{
    cleanup();
    event_base_free(base);
}

uint64_t NetworkConnCtxWithEvent::write(void *data, uint64_t data_len)
{
    int ret = evbuffer_add(bufferevent_get_output(bev), data, data_len);
    return ret;
}

NetworkConnCtxWithEvent::~NetworkConnCtxWithEvent()
{
    if (bev != nullptr)
        bufferevent_free(bev);

    if (peer_sock > 0)
        close(peer_sock);
}

NetworkConnCtxWithEventForUDP::~NetworkConnCtxWithEventForUDP()
{
    if (peer_sock > 0)
        close(peer_sock);
}

void NetworkConnCtxWithEventForUDP::setPeerSockAddr(struct sockaddr_in peer_addr)
{
    this->peer_addr = peer_addr;
}

uint64_t NetworkConnCtxWithEventForUDP::write(void *data, uint64_t data_len)
{
    return sendto(peer_sock, data, data_len, 0, (struct sockaddr *)&peer_addr, sizeof(struct sockaddr_in));
}

NetworkEndpointWithEvent::~NetworkEndpointWithEvent()
{
    if (e != nullptr)
        event_free(e);
}

void NetworkEndpointWithEvent::setEvent(struct event *e)
{
    this->e = e;
}