#include "discover_server.h"

struct event_base *base = event_base_new();
DiscoverServer *disconverServer = new DiscoverServer(base);

DiscoverServer::DiscoverServer(struct event_base *base)
{
    st = STATE_DISCOVERING;
    this->base = base;
}

DiscoverServer::~DiscoverServer()
{
}

void tcp_readcb(struct bufferevent *bev, void *ctx)
{
    printf("[DEBUG] TCP : receive msg!\n");
}

void tcp_writecb(struct bufferevent *bev, void *ctx)
{
    printf("[DEBUG] TCP : can write msg!\n");
}

void do_accept(evutil_socket_t listener, short event, void *ctx)
{
    struct event_base *base = (struct event_base *)ctx;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);

    int fd = accept(listener, (struct sockaddr *)&ss, &slen);
    assert(fd > 0 && fd < FD_SETSIZE);

    struct bufferevent *bev;
    evutil_make_socket_nonblocking(fd);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, tcp_readcb, tcp_writecb, nullptr, base);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void start_tcp_server(struct event_base *base)
{
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(tcp_sock > 0);
    evutil_make_socket_nonblocking(tcp_sock);
    int optval = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVER_SERVER_HTTP_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    assert(bind(tcp_sock, (struct sockaddr *)&addr, sizeof(addr)) == 0);

    int res = listen(tcp_sock, 100);
    if (res == -1)
    {
        fprintf(stderr, "[ERROR] TCP : %s \n", strerror(errno));
        exit(-1);
    }
    struct event *listener_event = event_new(base, tcp_sock, EV_READ | EV_PERSIST, do_accept, (void *)base);
    event_add(listener_event, nullptr);
}

void udp_readcb(evutil_socket_t fd, short events, void *ctx)
{
    struct event_base *base = (struct event_base *)ctx;

    struct cb_arg *arg = cb_arg_new(base);
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char data[4096] = {0};

    int receive = recvfrom(fd, data, 4096, 0, (struct sockaddr *)arg->target_addr, &addrlen);
    if (receive <= 0)
    {
        printf("[WARNING]: cannot receive anything !\n");
        return;
    }

    struct lan_discover_header *header = (lan_discover_header_t *)data;

    if (header->type == LAN_DISCOVER_TYPE_HELLO)
    {
        // 启动HTTPS Server
        if (disconverServer->st == STATE_DISCOVERING)
        {
            start_tcp_server(base);
        }
        disconverServer->st = STATE_SYNC_READY;
        uint16_t msg = DISCOVER_SERVER_HTTP_PORT;
        lan_discover_header_t reply_header = {LAN_DISCOVER_VER_0_1, LAN_DISCOVER_TYPE_HELLO_ACK, sizeof(msg)};

        evbuffer_add(arg->buf, &reply_header, sizeof(lan_discover_header_t));
        evbuffer_add(arg->buf, &msg, sizeof(msg));

        // todo(lutar) 检查并形成RESOURCE TABLE
    }

    struct event *write_e = event_new(base, fd, EV_WRITE, writecb, arg);
    event_add(write_e, nullptr);
}

void DiscoverServer::start()
{
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(udp_sock > 0);
    int optval = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    evutil_make_socket_nonblocking(udp_sock);

    struct sockaddr_in addr;
    addr.sin_port = htons(DISCOVER_SERVER_UDP_PORT);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    assert(bind(udp_sock, (struct sockaddr *)&addr, sizeof(addr)) >= 0);

    printf("listen: %d\n", DISCOVER_SERVER_UDP_PORT);

    struct event *read_e = event_new(base, udp_sock, EV_READ | EV_PERSIST, udp_readcb, base);
    event_add(read_e, nullptr);

    event_base_dispatch(base);
    free(read_e);
}

int main(int argc, char const *argv[])
{
    disconverServer->start();
    event_base_free(base);
    return 0;
}
