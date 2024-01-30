#include "discover_server.h"

DiscoverServer::DiscoverServer(struct event_base *base)
{
    st = STATE_DISCOVERING;
    this->base = base;
}

DiscoverServer::~DiscoverServer()
{
}

void *thread_cb(void *arg)
{
    // todo
    return nullptr;
}

void errorcb(struct bufferevent *bev, short error, void *ctx)
{
    if (error & BEV_EVENT_EOF)
    {
        /* connection has been closed, do any clean up here */
        /* ... */
    }
    else if (error & BEV_EVENT_ERROR)
    {
        /* check errno to see what error occurred */
        /* ... */
    }
    else if (error & BEV_EVENT_TIMEOUT)
    {
        /* must be a timeout event handle, handle it */
        /* ... */
    }
    bufferevent_free(bev);
}
void writecb(struct bufferevent *bev, void *arg)
{
    printf("To send is successufl!\n");
}

void readcb(struct bufferevent *bev, void *arg)
{
    printf("recive msg \n");
    struct DiscoverServer *dis = (DiscoverServer *)arg;
    struct evbuffer *input, *output;
    input = bufferevent_get_input(bev);
    output = bufferevent_get_output(bev);

    char buf[evbuffer_get_length(input)] = {0};
    evbuffer_copyout(input, buf, evbuffer_get_length(input));

    struct lan_discover_header *header = (lan_discover_header_t *)buf;

    if (header->type == LAN_DISCOVER_TYPE_HELLO)
    {
        // 启动HTTPS Server
        if (dis->st == STATE_DISCOVERING)
        {
            printf("[DEBUG] state is STATE_DISCOVERING \n");
            dis->st = STATE_SYNC_READY;
            uint16_t msg = DISCOVER_SERVER_HTTP_PORT;
            lan_discover_header_t reply_header = {LAN_DISCOVER_VER_0_1, LAN_DISCOVER_TYPE_HELLO_ACK, sizeof(msg)};
            evbuffer_add(output, &reply_header, sizeof(reply_header));
            evbuffer_add(output, &msg, sizeof(msg));
        }
        else
            printf("%s : %d", "unsupport type, do not reply \n", header->type);
    }
}

void do_accept(evutil_socket_t listener, short event, void *arg)
{
    struct DiscoverServer *dis = (DiscoverServer *)arg;
    struct event_base *base = dis->base;

    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);

    struct bufferevent *bev;
    bev = bufferevent_socket_new(base, listener, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, readcb, writecb, errorcb, arg);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
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

    struct event *listener_event = event_new(base, sock, EV_READ | EV_PERSIST, do_accept, (void *)this);

    event_add(listener_event, NULL);
    event_base_dispatch(base);
}

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    assert(base);

    DiscoverServer disconverServer(base);
    disconverServer.start();

    event_base_free(base);
    return 0;
}
