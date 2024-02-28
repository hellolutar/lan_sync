#include "sync_server.h"

#ifndef RELEASE
struct event_base *base = event_base_new();
SyncServer *sync_server = new SyncServer(base);
#else
extern SyncServer *sync_server;
#endif

static void srv_tcp_readcb(struct bufferevent *bev, void *ctx)
{
    struct evbuffer *in = bufferevent_get_input(bev);

    uint32_t recvLen = evbuffer_get_length(in);
    if (recvLen == 0)
        return;

    sync_server->handleTcpMsg(bev);
}

static void srv_tcp_writecb(struct bufferevent *bev, void *ctx)
{
    // LOG_DEBUG("TCP : can write msg!");
    LOG_INFO("[SYNC CLI] call tcp_writecb!");
}

static void srv_do_accept(evutil_socket_t listener, short event, void *ctx)
{
    struct event_base *base = (struct event_base *)ctx;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);

    int fd = accept(listener, (struct sockaddr *)&ss, &slen);
    assert(fd > 0 && fd < FD_SETSIZE);

    struct bufferevent *bev;
    evutil_make_socket_nonblocking(fd);
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, srv_tcp_readcb, srv_tcp_writecb, nullptr, base);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

// TODO(lutar, 20240219)  优化有限状态机
static void srv_udp_readcb(evutil_socket_t fd, short events, void *ctx)
{
    struct event_base *base = (struct event_base *)ctx;

    struct sockaddr_in target_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char data[4096] = {0};

    uint32_t receive = recvfrom(fd, data, 4096, 0, (sockaddr *)&target_addr, &addrlen);
    if (receive <= 0)
    {
        LOG_WARN("[SYNC SER] cannot receive anything !");
        return;
    }

#ifdef RELEASE
    auto ports = LocalPort::query();
    if (LocalPort::existIp(ports, target_addr.sin_addr))
        return;
#endif

    sync_server->handleUdpMsg(target_addr, data, 4096);
}

void SyncServer::handleUdpMsg(struct sockaddr_in target_addr, char *data, int data_len)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (header->type == LAN_SYNC_TYPE_HELLO)
    {
        LOG_DEBUG("[SYNC SER] receive pkt : {}", SERVICE_NAME_DISCOVER_HELLO);
        // 启动TCP Server
        if (st == STATE_DISCOVERING)
            start_tcp_server(base);
        st = STATE_SYNC_READY;

        LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_HELLO_ACK);
        pkt.addXheader(XHEADER_TCPPORT, to_string(DISCOVER_SERVER_TCP_PORT));

        struct evbuffer *buf = evbuffer_new();
        pkt.write(buf);

        udp_cli *cli = new udp_cli(udp_sock, base, target_addr);
        cli->send(buf);
    }
    else
    {
        char *ip = inet_ntoa(target_addr.sin_addr);

        LOG_WARN("[SYNC SER] receive pkt[{}:{}] : the type is unsupport : {}", ip, ntohs(target_addr.sin_port), header->type);
    }
}

SyncServer::SyncServer(struct event_base *base)
{
    st = STATE_DISCOVERING;
    this->base = base;
}

// SyncServer::~SyncServer()
// {
// }

void SyncServer::handleLanSyncGetTableIndex(struct bufferevent *bev, lan_sync_header_t *try_header, uint32_t recvLen)
{
    struct evbuffer *in = bufferevent_get_input(bev);
    struct evbuffer *out = bufferevent_get_output(bev);

    char useless[1024];
    uint16_t total_len = ntohl(try_header->total_len);
    if (recvLen < total_len)
    {
        return;
    }

    evbuffer_remove(in, useless, total_len);

    vector<struct Resource *> table = sync_server->rm.getTable();

    uint32_t data_len = sizeof(struct Resource) * table.size();
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);

    struct Resource *data = resource_convert_vec_to_arr(table);
    pkt.setData(data, data_len);
    pkt.write(bev);
    free(data);

    LOG_INFO("[SYNC SER] [{}] : entry num: {} ", SERVICE_NAME_REPLY_TABLE_INDEX, table.size());
}

void SyncServer::replyResource(struct bufferevent *bev, LanSyncPkt &pkt)
{
    struct evbuffer *in = bufferevent_get_input(bev);
    struct evbuffer *out = bufferevent_get_output(bev);

    string xhd_uri = pkt.queryXheader(XHEADER_URI);
    string range_str = pkt.queryXheader(XHEADER_RANGE);
    Range range(range_str);

    char *uri = xhd_uri.data();
    LOG_INFO("[SYNC SER] [{}] : uri[{}] ", SERVICE_NAME_REQ_RESOURCE, uri);

    const struct Resource *rs = sync_server->rm.queryByUri(uri);
    if (rs == nullptr)
    {
        return;
    }

    uint64_t once_read_max_num = SIZE_1KB * 1000;
    int fd = open(rs->path, O_RDONLY);
    off_t currpos = lseek(fd, range.getStartPos(), SEEK_SET);

    uint64_t readed = range.getStartPos();
    uint64_t end = range.getStartPos() + (range.getSize() == 0 ? rs->size : range.getSize());
    char *data = (char *)malloc(once_read_max_num);
    while (readed < end)
    {
        memset(data, 0, once_read_max_num);
        uint64_t start_at = readed;
        LanSyncPkt reply_pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_RESOURCE);

        // 读取文件内容
        uint64_t want_to_read_num = min(once_read_max_num, (rs->size - readed));
        readed += read(fd, data, want_to_read_num);
        uint64_t cur_readed = readed - start_at;

        if (readed != end)
        {
            ContentRange cr(start_at, cur_readed, rs->size, false);
            reply_pkt.addXheader(XHEADER_CONTENT_RANGE, cr.to_string());
        }
        else
        {
            ContentRange cr(start_at, cur_readed, rs->size, true);
            reply_pkt.addXheader(XHEADER_CONTENT_RANGE, cr.to_string());
            reply_pkt.addXheader(XHEADER_HASH, rs->hash);
        }
        reply_pkt.addXheader(XHEADER_URI, rs->uri);

        reply_pkt.setData(data, cur_readed);
        reply_pkt.write(bev);
    }
    free(data);
    close(fd);

    LOG_DEBUG("[SYNC SER] [{}] : uri[{}] file size:{} ", SERVICE_NAME_REPLY_REQ_RESOURCE, uri, readed);
}

void SyncServer::handleLanSyncGetResource(struct bufferevent *bev, lan_sync_header_t *try_header, uint32_t recvLen)
{
    struct evbuffer *in = bufferevent_get_input(bev);
    struct evbuffer *out = bufferevent_get_output(bev);

    uint32_t total_len = ntohl(try_header->total_len);
    if (recvLen < total_len)
    {
        return;
    }

    char *bufp = (char *)malloc(total_len);
    memset(bufp, 0, total_len);

    recvLen = evbuffer_remove(in, bufp, total_len);
    assert(recvLen == total_len);

    lan_sync_header_t *header = (lan_sync_header_t *)bufp;
    LanSyncPkt pkt(header);

    replyResource(bev, pkt);

    free(bufp);
}

void SyncServer::handleTcpMsg(struct bufferevent *bev)
{
    struct evbuffer *in = bufferevent_get_input(bev);
    struct evbuffer *out = bufferevent_get_output(bev);

    uint32_t recvLen = evbuffer_get_length(in);
    char buf[LEN_LAN_SYNC_HEADER_T + 1] = {0};
    evbuffer_copyout(in, buf, LEN_LAN_SYNC_HEADER_T);
    lan_sync_header_t *try_header = (lan_sync_header_t *)buf;
    if (try_header->type == LAN_SYNC_TYPE_GET_TABLE_INDEX)
    {
        LOG_INFO("[SYNC SER] receive pkt: {}", SERVICE_NAME_REQ_TABLE_INDEX);
        handleLanSyncGetTableIndex(bev, try_header, recvLen);
    }
    else if (try_header->type == LAN_SYNC_TYPE_GET_RESOURCE)
    {
        LOG_INFO("[SYNC SER] receive pkt: {}", SERVICE_NAME_REQ_RESOURCE);
        handleLanSyncGetResource(bev, try_header, recvLen);
    }
    else
    {
        LOG_INFO("[SYNC SER] : receive pkt: the type is unsupport!");
    }
}

void SyncServer::start_tcp_server(struct event_base *base)
{
    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(tcp_sock > 0);
    evutil_make_socket_nonblocking(tcp_sock);
    int optval = 1;
    setsockopt(tcp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(tcp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    setsockopt(tcp_sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int));

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
    struct event *listener_event = event_new(base, tcp_sock, EV_READ | EV_PERSIST, srv_do_accept, (void *)base);
    event_add(listener_event, nullptr);
}

void SyncServer::start()
{
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
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

    LOG_INFO("[SYNC SER] UDP listen: {}", DISCOVER_SERVER_UDP_PORT);

    struct event *read_e = event_new(base, udp_sock, EV_READ | EV_PERSIST, srv_udp_readcb, base);
    event_add(read_e, nullptr);

    event_base_dispatch(base);
    free(read_e);
}

#ifndef RELEASE
int main(int argc, char const *argv[])
{
    configlog();
    sync_server->start();
    event_base_free(base);
    return 0;
}
#endif