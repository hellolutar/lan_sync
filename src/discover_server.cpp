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

void handleLanSyncGetTableIndex(struct evbuffer *in, struct evbuffer *out, lan_sync_header_t *try_header, int recvLen)
{
    char useless[1024];
    evbuffer_remove(in, useless, try_header->header_len);

    vector<struct Resource *> table = disconverServer->rm.getTable();

    int data_len = sizeof(struct Resource) * table.size();
    lan_sync_header_t *header = lan_sync_header_new(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX); // free in lan_sync_encapsulate --> evbuffer_cb_for_free
    struct Resource *data = lan_sync_parseTableToData(table);
    header = lan_sync_header_set_data(header, data, data_len);
    lan_sync_encapsulate(out, header);
    free(data);

    LOG_INFO("[SYNC SER] [{}] : entry num: {} ", SERVICE_NAME_REPLY_TABLE_INDEX, table.size());
}

void replyResource(struct evbuffer *out, char *uri)
{
    const struct Resource *rs = disconverServer->rm.queryByUri(uri);
    if (rs == nullptr)
    {
        return;
    }

    lan_sync_header_t *header = lan_sync_header_new(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_RESOURCE);

    string uristr(uri);
    header = lan_sync_header_add_xheader(header, XHEADER_URI, uristr);

    string hash(rs->hash);
    header = lan_sync_header_add_xheader(header, XHEADER_HASH, hash);

    // 读取文件内容
    int fd = open(rs->path, O_RDONLY);
    char *data = (char *)malloc(rs->size); // free in lan_sync_encapsulate --> evbuffer_cb_for_free

    uint64_t readed = read(fd, data, rs->size);
    while (readed != rs->size)
    {
        readed += read(fd, data, rs->size);
    }
    close(fd);

    header = lan_sync_header_set_data(header, data, readed);
    lan_sync_encapsulate(out, header);
    free(data);
    LOG_DEBUG("[SYNC SER] [{}] : uri[{}] file size:{} ", SERVICE_NAME_REPLY_REQ_RESOURCE, uri, readed);
}

void handleLanSyncGetResource(struct evbuffer *in, struct evbuffer *out, lan_sync_header_t *try_header, int recvLen)
{
    int total_len = try_header->total_len;
    if (recvLen < total_len)
    {
        return;
    }

    char *bufp = (char *)malloc(total_len);
    memset(bufp, 0, total_len);

    recvLen = evbuffer_remove(in, bufp, total_len);
    assert(recvLen == total_len);

    lan_sync_header_t *header = (lan_sync_header_t *)bufp;
    string xhd_uri = lan_sync_header_query_xheader(header, XHEADER_URI);
    char *reqUri = xhd_uri.data();
    LOG_INFO("[SYNC SER] [{}] : uri[{}] ", SERVICE_NAME_REQ_RESOURCE, reqUri);

    replyResource(out, reqUri);

    free(bufp);
}

void tcp_readcb(struct bufferevent *bev, void *ctx)
{
    struct evbuffer *in = bufferevent_get_input(bev);
    struct evbuffer *out = bufferevent_get_output(bev);

    int recvLen = evbuffer_get_length(in);
    char buf[lan_sync_header_len + 1] = {0};
    evbuffer_copyout(in, buf, lan_sync_header_len);
    lan_sync_header_t *try_header = (lan_sync_header_t *)buf;
    if (try_header->type == LAN_SYNC_TYPE_GET_TABLE_INDEX)
    {
        LOG_INFO("[SYNC SER] receive pkt: {}", SERVICE_NAME_REQ_TABLE_INDEX);
        handleLanSyncGetTableIndex(in, out, try_header, recvLen);
    }
    else if (try_header->type == LAN_SYNC_TYPE_GET_RESOURCE)
    {
        LOG_INFO("[SYNC SER] receive pkt: {}", SERVICE_NAME_REQ_RESOURCE);
        handleLanSyncGetResource(in, out, try_header, recvLen);
    }
    else
    {
        LOG_INFO("[SYNC SER] : receive pkt: the type is unsupport!");
    }
}

void tcp_writecb(struct bufferevent *bev, void *ctx)
{
    // LOG_DEBUG("TCP : can write msg!");
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
        LOG_ERROR("[SYNC SER] : {} ", strerror(errno));
        exit(-1);
    }
    LOG_INFO("[SYNC SER] TCP listen : {}", DISCOVER_SERVER_TCP_PORT);
    struct event *listener_event = event_new(base, tcp_sock, EV_READ | EV_PERSIST, do_accept, (void *)base);
    event_add(listener_event, nullptr);
}

// TODO(lutar, 20240219)  优化有限状态机
void udp_readcb(evutil_socket_t fd, short events, void *ctx)
{
    struct event_base *base = (struct event_base *)ctx;

    struct cb_arg *arg = cb_arg_new(base);
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char data[4096] = {0};

    int receive = recvfrom(fd, data, 4096, 0, (struct sockaddr *)arg->target_addr, &addrlen);
    if (receive <= 0)
    {
        LOG_WARN("[UDP] cannot receive anything !");
        return;
    }

    struct lan_discover_header *header = (lan_discover_header_t *)data;

    if (header->type == LAN_DISCOVER_TYPE_HELLO)
    {
        LOG_INFO("[UDP] receive pkt : {}", SERVICE_NAME_DISCOVER_HELLO);
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
    else
    {
        LOG_WARN("[UDP] receive pkt : the type is unsupport", SERVICE_NAME_DISCOVER_HELLO);
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

    LOG_INFO("[UDP] UDP listen: {}", DISCOVER_SERVER_UDP_PORT);

    struct event *read_e = event_new(base, udp_sock, EV_READ | EV_PERSIST, udp_readcb, base);
    event_add(read_e, nullptr);

    event_base_dispatch(base);
    free(read_e);
}

int main(int argc, char const *argv[])
{
    configlog();
    disconverServer->start();
    event_base_free(base);
    return 0;
}
