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


void readcb(evutil_socket_t fd, short events, void *ctx)
{
    struct event_base *base = (struct event_base *)ctx;

    struct cb_arg *arg = cb_arg_new(base);
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char data[4096] = {0};

    int receive = recvfrom(fd, data, 4096, 0, (struct sockaddr *)arg->target_addr, &addrlen);
    if (receive <= 0){
        printf("[WARNING]: cannot receive anything !\n");
        return;
    }

    struct lan_discover_header *header = (lan_discover_header_t *)data;

    if (header->type == LAN_DISCOVER_TYPE_HELLO)
    {
        // 启动HTTPS Server
        if (disconverServer->st == STATE_DISCOVERING)
        {
            printf("[DEBUG] state is STATE_DISCOVERING \n");
            disconverServer->st = STATE_SYNC_READY;
            uint16_t msg = DISCOVER_SERVER_HTTP_PORT;
            lan_discover_header_t reply_header = {LAN_DISCOVER_VER_0_1, LAN_DISCOVER_TYPE_HELLO_ACK, sizeof(msg)};

            evbuffer_add(arg->buf, &reply_header, sizeof(lan_discover_header_t));
            evbuffer_add(arg->buf, &msg, sizeof(msg));
        }
        else
            printf("%s : %d", "unsupport type, do not reply \n", header->type);
    }

    struct event *write_e = event_new(base, fd, EV_WRITE, writecb, arg);
    event_add(write_e, NULL);
}

void DiscoverServer::start()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock > 0);
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    evutil_make_socket_nonblocking(sock);

    struct sockaddr_in addr;
    addr.sin_port = htons(DISCOVER_SERVER_UDP_PORT);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    assert(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) >= 0);

    printf("listen: %d\n", DISCOVER_SERVER_UDP_PORT);

    struct event *read_e = event_new(base, sock, EV_READ | EV_PERSIST, readcb, base);
    event_add(read_e, NULL);

    event_base_dispatch(base);
    free(read_e);
}

int main(int argc, char const *argv[])
{
    disconverServer->start();
    event_base_free(base);
    return 0;
}
