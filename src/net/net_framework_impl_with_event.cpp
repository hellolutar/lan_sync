#include "net_framework_impl_with_event.h"

using namespace std;

struct event_base *NetFrameworkImplWithEvent::base;
std::vector<event *> NetFrameworkImplWithEvent::events; // only persist event need to add
std::vector<NetworkConnCtx *> NetFrameworkImplWithEvent::tcp_ctx;
std::vector<NetworkConnCtx *> NetFrameworkImplWithEvent::udp_ctx;

void NetFrameworkImplWithEvent::init(struct event_base *eb)
{
    base = eb;
}

void NetFrameworkImplWithEvent::init_check()
{
    if (base == nullptr)
    {
        LOG_ERROR("please NetFrameworkImplWithEvent::init()");
        exit(-1);
    }
}

void NetFrameworkImplWithEvent::event_cb(struct bufferevent *bev, short events, void *data)
{
    switch (events)
    {
    case BEV_EVENT_EOF:
        LOG_ERROR("NetFrameworkImplWithEvent::event_cb : BEV_EVENT_EOF : {}", strerror(errno));
        break;
    case BEV_EVENT_ERROR:
        LOG_ERROR("NetFrameworkImplWithEvent::event_cb : BEV_EVENT_ERROR : {}", strerror(errno));
        break;
    case BEV_EVENT_TIMEOUT:
        LOG_ERROR("NetFrameworkImplWithEvent::event_cb : BEV_EVENT_TIMEOUT : {}", strerror(errno));
        break;
    case BEV_EVENT_CONNECTED:
        LOG_ERROR("NetFrameworkImplWithEvent::event_cb : BEV_EVENT_CONNECTED : {}", strerror(errno));
        break;
    case BEV_EVENT_READING:
        LOG_ERROR("NetFrameworkImplWithEvent::event_cb : BEV_EVENT_READING : {}", strerror(errno));
        break;
    case BEV_EVENT_WRITING:
        LOG_ERROR("NetFrameworkImplWithEvent::event_cb : BEV_EVENT_WRITING : {}", strerror(errno));
        break;
    default:
        LOG_ERROR("NetFrameworkImplWithEvent::event_cb : OTHER : {}", strerror(errno));
        break;
    }

    NetworkConnCtx *nctx = (NetworkConnCtx *)data;
    nctx->setActive(false);
    for (auto iter = tcp_ctx.begin(); iter != tcp_ctx.end(); iter++)
    {
        if ((*iter) == nctx)
        {
            LOG_INFO("NetFrameworkImplWithEvent::event_cb : remvoe tcp_ctx : {}", nctx->getPeer().str());
            tcp_ctx.erase(iter);
            break;
        }
    }
}

void NetFrameworkImplWithEvent::write_cb(struct bufferevent *bev, void *data)
{
}

void NetFrameworkImplWithEvent::read_cb(struct bufferevent *bev, void *arg)
{
    struct evbuffer *in = bufferevent_get_input(bev);

    uint32_t recvLen = evbuffer_get_length(in);
    if (recvLen == 0)
        return;

    uint64_t data_len = evbuffer_get_length(in);
    uint8_t *data = new uint8_t[data_len];
    evbuffer_copyout(in, data, data_len);

    NetworkConnCtxWithEvent *ctx = (NetworkConnCtxWithEvent *)arg;
    NetAbility *ne = ctx->getNetworkEndpoint();

    uint64_t ne_wanto_extra_len = 0;
    ne->isExtraAllDataNow((void *)data, data_len, ne_wanto_extra_len);
    if (ne_wanto_extra_len > 0)
    {
        memset(data, 0, ne_wanto_extra_len);
        ne_wanto_extra_len = evbuffer_remove(in, data, ne_wanto_extra_len);

        ne->recv((void *)data, ne_wanto_extra_len, ctx);
    }
    delete[] data;
}

void NetFrameworkImplWithEvent::tcp_accept(evutil_socket_t listener, short event, void *ctx)
{
    NetAbility *ne = (NetAbility *)ctx;

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

    NetworkConnCtxWithEvent *nctx = new NetworkConnCtxWithEvent(&tcp_ctx, ne, bev, peer_sock, NetAddr::fromBe(peer));
    tcp_ctx.push_back(nctx);
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, nctx);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void NetFrameworkImplWithEvent::addTcpServer(NetAbilityImplWithEvent *ne)
{
    init_check();

    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0)
    {
        LOG_ERROR("NetFrameworkImplWithEvent::addTcpServer : {}", strerror(errno));
        shutdown();
        return;
    }

    evutil_make_socket_nonblocking(tcp_sock);
    int optval = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    setsockopt(tcp_sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int));

    sockaddr_in be_addr = ne->getAddr().getBeAddr();
    if (bind(tcp_sock, (struct sockaddr *)&be_addr, sizeof(struct sockaddr_in)) != 0)
    {
        LOG_ERROR("NetFrameworkImplWithEvent::addTcpServer : {}", strerror(errno));
        shutdown();
        return;
    }

    int res = listen(tcp_sock, 100);
    if (res == -1)
    {
        LOG_ERROR("NetFrameworkImplWithEvent::addTcpServer : {}", strerror(errno));
        shutdown();
        return;
    }
    ne->setSock(tcp_sock);

    LOG_INFO("TCP listen: {}", ne->getAddr().str().data());

    struct event *accept_event_persist = event_new(base, tcp_sock, EV_READ | EV_PERSIST, tcp_accept, (void *)ne);
    event_add(accept_event_persist, nullptr);

    ne->setEvent(accept_event_persist);
}

void NetFrameworkImplWithEvent::udp_read_cb(evutil_socket_t fd, short events, void *arg)
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

    nctx->setNetAddr(NetAddr::fromBe(target_addr));

    NetAbility *ne = nctx->getNetworkEndpoint();
    ne->recv((void *)data, receive, nctx);
}

void NetFrameworkImplWithEvent::addUdpServer(NetAbilityImplWithEvent *ne)
{
    init_check();

    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock <= 0)
    {
        LOG_ERROR("NetFrameworkImplWithEvent::addUdpServer : {}", strerror(errno));
        shutdown();
        return;
    }

    int optval = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    evutil_make_socket_nonblocking(udp_sock);

    auto be_addr = ne->getAddr().getBeAddr();

    int udpret = bind(udp_sock, (struct sockaddr *)&be_addr, sizeof(struct sockaddr_in));
    if (udpret != 0)
    {
        LOG_ERROR("NetFrameworkImplWithEvent::addUdpServer : {}", strerror(errno));
        shutdown();
        return;
    }

    ne->setSock(udp_sock);

    LOG_INFO("UDP listen: {}", ne->getAddr().str().data());

    NetAddr addr_will_be_udpate_follow;
    NetworkConnCtxWithEventForUDP *nctx = new NetworkConnCtxWithEventForUDP(&udp_ctx, ne, udp_sock, addr_will_be_udpate_follow); // udp_sock close many time: 1.NetworkEndpointWithEvent; 2.NetworkConnCtx
    udp_ctx.push_back(nctx);

    struct event *read_e = event_new(base, udp_sock, EV_READ | EV_PERSIST, udp_read_cb, nctx);
    event_add(read_e, nullptr);
    ne->setEvent(read_e);
}

NetworkConnCtx *NetFrameworkImplWithEvent::connectWithTcp(NetAbilityImplWithEvent *peer_ne)
{
    init_check();

    LOG_INFO("TCP connect : {}", peer_ne->getAddr().str());

    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int peer_sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in be_addr = peer_ne->getAddr().getBeAddr();
    int ret = connect(peer_sock, (struct sockaddr *)&be_addr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        LOG_ERROR("NetFrameworkImplWithEvent::connectWithTcp {} : {}", peer_ne->getAddr().str(), strerror(errno));
        delete peer_ne;
        return nullptr;
    }

    struct bufferevent *bev = bufferevent_socket_new(base, peer_sock, BEV_OPT_CLOSE_ON_FREE);

    NetworkConnCtxWithEvent *nctx = new NetworkConnCtxWithEvent(&tcp_ctx, peer_ne, bev, peer_sock, peer_ne->getAddr());
    tcp_ctx.push_back(nctx);
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, nctx);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    return nctx; // todo determine delete
}

NetworkConnCtx *NetFrameworkImplWithEvent::connectWithUdp(NetAbilityImplWithEvent *peer_ne)
{
    init_check();

    int peer_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (peer_sock <= 0)
    {
        LOG_ERROR("NetFrameworkImplWithEvent::connectWithUdp {} : {}", peer_ne->getAddr().str(), strerror(errno));
        delete peer_ne;
        return nullptr;
    }

    int optval = 1; // 这个值一定要设置，否则可能导致sendto()失败
    setsockopt(peer_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(peer_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(peer_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    evutil_make_socket_nonblocking(peer_sock);

    LOG_INFO("UDP connect : {}", peer_ne->getAddr().str().data());

    NetworkConnCtxWithEventForUDP *nctx = new NetworkConnCtxWithEventForUDP(&udp_ctx, peer_ne, peer_sock, peer_ne->getAddr()); // todo &peer_ne maybe a problem
    udp_ctx.push_back(nctx);

    struct event *read_e = event_new(base, peer_sock, EV_READ | EV_PERSIST, udp_read_cb, nctx);
    event_add(read_e, nullptr);
    peer_ne->setEvent(read_e);

    return nctx;
}

void NetFrameworkImplWithEvent::run()
{
    init_check();

    event_base_dispatch(base);
}

void NetFrameworkImplWithEvent::shutdown()
{
    event_base_loopbreak(base);
    cleanup();
}

void NetFrameworkImplWithEvent::cleanup()
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

void NetFrameworkImplWithEvent::free()
{
    cleanup();
    event_base_free(base);
}

uint64_t NetworkConnCtxWithEvent::write(void *data, uint64_t data_len)
{
    if (!active)
    {
        delete this;
        throw NetworkConnCtxException();
    }

    int ret = evbuffer_add(bufferevent_get_output(bev), data, data_len);
    LOG_DEBUG("NetworkConnCtxWithEvent::write: {}, sent [{}]", this->peer.str().data(), data_len);
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

uint64_t NetworkConnCtxWithEventForUDP::write(void *data, uint64_t data_len)
{
    LOG_DEBUG("[UDP] sendto [{}], len:{}", this->peer.str().data(), data_len);
    sockaddr_in be_addr = peer.getBeAddr();
    return sendto(peer_sock, data, data_len, 0, (struct sockaddr *)&be_addr, sizeof(struct sockaddr_in));
}

NetAbilityImplWithEvent::~NetAbilityImplWithEvent()
{
    if (e != nullptr)
        event_free(e);
}

void NetAbilityImplWithEvent::setEvent(struct event *e)
{
    this->e = e;
}