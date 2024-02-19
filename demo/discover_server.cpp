#include "discover_server.h"

struct event_base *base = event_base_new();
DiscoverServer *disconverServer = new DiscoverServer(base);

DiscoverServer::DiscoverServer(struct event_base *base)
{
    st = STATE_DISCOVERING;
    this->base = base;
}

// DiscoverServer::~DiscoverServer()
// {
// }

void tcp_readcb(struct bufferevent *bev, void *ctx)
{
    printf("[DEBUG] [TCP] : receive msg!\n");
    struct evbuffer *in = bufferevent_get_input(bev);
    int len = evbuffer_get_length(in);
    char buf[2048] = {0};
    int ret = evbuffer_remove(in, buf, len);
    if (ret >= 0)
    {
        // TODO  取data中的数据时：数据量过大，无法一次取完，该如何处理。
        lan_sync_header_t *header = (lan_sync_header_t *)buf;
        if (header->type == LAN_SYNC_TYPE_GET_TABLE_INDEX)
        {
            printf("[DEBUG] [TCP] cli req table index!\n");
            vector<struct Resource *> table = disconverServer->rm.getTable();
            // 将table 转为 data
            struct Resource *data = lan_sync_parseTableToData(table);

            struct evbuffer *out = bufferevent_get_output(bev);
            lan_sync_header_t header = {
                .version = LAN_SYNC_VER_0_1,
                .type = LAN_SYNC_TYPE_REPLY_TABLE_INDEX,
                .header_len = (uint16_t)sizeof(lan_sync_header_t),
                .data_len = sizeof(struct Resource) * table.size()};
            lan_sync_encapsulate(out, header, data, header.data_len);
        }
    }
    else
    {
        printf("[WARNING] [TCP] cannot recive data from evbuffer!");
    }
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
    addr.sin_port = htons(DISCOVER_SERVER_TCP_PORT);
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
        // 启动TCP Server
        if (disconverServer->st == STATE_DISCOVERING)
        {
            start_tcp_server(base);
        }
        disconverServer->st = STATE_SYNC_READY;
        uint16_t msg = DISCOVER_SERVER_TCP_PORT;
        lan_discover_header_t reply_header = {LAN_DISCOVER_VER_0_1, LAN_DISCOVER_TYPE_HELLO_ACK, sizeof(msg)};

        evbuffer_add(arg->buf, &reply_header, sizeof(lan_discover_header_t));
        evbuffer_add(arg->buf, &msg, sizeof(msg));
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

    printf("[UDP] listen: %d\n", DISCOVER_SERVER_UDP_PORT);

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
