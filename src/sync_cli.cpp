
#include "sync_cli.h"

#ifndef RELEASE
struct event_base *base = event_base_new();
SyncCli *sync_cli = new SyncCli(base);
#else
extern SyncCli *sync_cli;
#endif

static void req_resource_periodically_timeout(evutil_socket_t, short, void *arg)
{
    sync_cli->syncResource();
}

static void req_table_index_timeout_cb(evutil_socket_t, short, void *arg)
{
    sync_cli->reqTableIndex();
}

static void tcp_readcb(struct bufferevent *bev, void *ctx)
{
    struct evbuffer *in = bufferevent_get_input(bev);

    uint32_t recvLen = evbuffer_get_length(in);
    if (recvLen == 0)
        return;
    sync_cli->handleTcpMsg(bev);
}

static void tcp_writecb(struct bufferevent *bev, void *ctx)
{
    // LOG_INFO("[SYNC CLI] call tcp_writecb!");
}

static void tcp_event_cb(struct bufferevent *bev, short what, void *ctx)
{
    if (what == BEV_EVENT_ERROR)
    {
        LOG_WARN("[SYNC CLI] tcp_event_cb [BEV_EVENT_ERROR]");
    }
    else if (what == BEV_EVENT_EOF)
    {
        LOG_WARN("[SYNC CLI] tcp_event_cb [BEV_EVENT_EOF]");
    }
    else if (what == BEV_EVENT_TIMEOUT)
    {
        LOG_WARN("[SYNC CLI] tcp_event_cb [BEV_EVENT_TIMEOUT]");
    }
    else if (BEV_EVENT_CONNECTED)
    {
        // socket disconnected
        // todo remove  event in config_req_table_index_periodically
        // inet_ntoa()
        LOG_WARN("[SYNC CLI] tcp_event_cb [BEV_EVENT_CONNECTED] REASON: {}", evutil_socket_error_to_string(errno));
        sync_cli->delTcpSessionByBufevent(bev);
    }
    else if (BEV_EVENT_READING)
    {
        LOG_WARN("[SYNC CLI] tcp_event_cb [BEV_EVENT_READING]");
    }
    else if (BEV_EVENT_WRITING)
    {
        LOG_WARN("[SYNC CLI] tcp_event_cb [BEV_EVENT_WRITING]");
    }
    else
    {
        LOG_WARN("[SYNC CLI] tcp_event_cb  what{} ", what);
    }
}

static void udp_readcb(evutil_socket_t fd, short events, void *ctx)
{
    struct sockaddr_in target_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char data[4096] = {0};

    uint32_t receive = recvfrom(fd, data, 4096, 0, (struct sockaddr *)&target_addr, &addrlen);
    if (receive <= 0 || receive < LEN_LAN_SYNC_HEADER_T)
    {
        LOG_WARN("warning: cannot receive anything !");
        return;
    }

#ifdef RELEASE
    auto ports = LocalPort::query();
    if (LocalPort::existIp(ports, target_addr.sin_addr))
        return;
#endif

    sync_cli->handleUdpMsg(target_addr, data, 4096);
}

map<in_addr_t, struct bufferevent *> SyncCli::getTcpTable()
{
    return tcpTable;
}

void SyncCli::addTcpSession(in_addr_t target_addr, struct bufferevent *bev)
{
    tcpTable[target_addr] = bev;
}

void SyncCli::delTcpSessionByBufevent(struct bufferevent *e)
{
    for (auto i = tcpTable.begin(); i != tcpTable.end(); i++)
    {
        auto target_addr = (*i).first;
        auto bev = (*i).second;
        if (bev == e)
        {
            tcpTable.erase(i);
            break;
        }
    }
}

bool SyncCli::existTcpSessionByBufevent(struct bufferevent *e)
{
    for (auto i = tcpTable.begin(); i != tcpTable.end(); i++)
    {
        auto target_addr = (*i).first;
        auto bev = (*i).second;
        if (bev == e)
        {
            return true;
        }
    }
    return false;
}

bool SyncCli::existTcpSessionByAddr(in_addr_t peer)
{
    for (auto i = tcpTable.begin(); i != tcpTable.end(); i++)
    {
        auto target_addr = (*i).first;
        auto bev = (*i).second;
        if (target_addr == peer)
        {
            return true;
        }
    }
    return false;
}

SyncCli::SyncCli(struct event_base *base)
{
    assert(base);

    this->st = STATE_DISCOVERING;
    this->base = base;
}

SyncCli::~SyncCli()
{
}

void SyncCli::appendSyncTable(struct Resource *table, struct bufferevent *bev, uint64_t res_num)
{
    map<string, Resource> total_table;

    for (size_t i = 0; i < res_num; i++)
    {
        if (strlen(table[i].uri) == 0)
            continue;

        LOG_DEBUG(" > uri: {}", table[i].uri);
        total_table[table[i].name] = table[i];
    }
    LOG_DEBUG("");
    if (total_table.size() == 0)
        return;

    vector<struct Resource *> local_table = sync_cli->rm.getTable();
    for (size_t i = 0; i < local_table.size(); i++)
    {
        struct Resource *local_rs = local_table[i];
        struct Resource rs = total_table[local_rs->name];
        if (strlen(rs.name) == 0 || rs.size < local_rs->size)
        {
            // I have the resource or my resource should sync to peer.
            total_table.erase(local_rs->name);
            continue;
        }
        if (rs.size == local_rs->size)
        {
            if (compareChar(rs.hash, local_rs->hash, strlen(rs.hash)))
                total_table.erase(local_rs->name);
            else
                LOG_WARN("[SYNC CLI] [{}] size is euqal, but hash is not equal\npeer:{}\nmine:{}", local_rs->uri, rs.hash, local_rs->hash);
        }
    }

    for (auto iter = total_table.begin(); iter != total_table.end(); iter++)
    {
        Resource rs = iter->second;
        LOG_DEBUG("[SYNC CLI] add uri to sync list : uri[{}]", rs.uri);
        sync_cli->addSyncResource(WantSyncResource_new(bev, rs.uri, PENDING, rs.size)); // free in delSyncResource
    }
}
void SyncCli::handleLanSyncReplyTableIndex(struct bufferevent *bev, lan_sync_header_t *try_header, int recvLen)
{
    struct evbuffer *in, *out;
    in = bufferevent_get_input(bev);
    out = bufferevent_get_output(bev);

    uint32_t total_len = ntohl(try_header->total_len);
    if (recvLen < total_len)
    {
        return;
    }
    LOG_INFO("[SYNC CLI] recive {}!", SERVICE_NAME_REPLY_TABLE_INDEX);

    char *bufp = (char *)malloc(total_len);
    memset(bufp, 0, total_len);

    recvLen = evbuffer_remove(in, bufp, total_len);
    assert(recvLen == total_len);

    lan_sync_header_t *header = (lan_sync_header_t *)bufp;
    LanSyncPkt pkt(header);

    // 提取记录数量
    uint32_t data_len = pkt.getTotalLen() - pkt.getHeaderLen();
    uint64_t res_num = data_len / sizeof(struct Resource);

    struct Resource *table = (struct Resource *)pkt.getData();

    appendSyncTable(table, bev, res_num);

    free(bufp);
}

uint64_t SyncCli::writeFile(string pathstr, LanSyncPkt &pkt)
{
    string content_range_str = pkt.queryXheader(XHEADER_CONTENT_RANGE);
    ContentRange cr(content_range_str);

    IoUtil io;
    return io.writeFile(pathstr, cr.getStartPos(), pkt.getData(), cr.getSize());
}

void SyncCli::handleLanSyncReplyResource(struct bufferevent *bev, lan_sync_header_t *try_header, int recvLen)
{

    struct evbuffer *in, *out;
    in = bufferevent_get_input(bev);
    out = bufferevent_get_output(bev);

    uint32_t total_len = ntohl(try_header->total_len);
    if (recvLen < total_len)
    {
        return;
    }
    LOG_INFO("[SYNC CLI] [{}] recive [{}]!", SERVICE_NAME_REQ_RESOURCE, SERVICE_NAME_REPLY_REQ_RESOURCE);

    char *bufp = (char *)malloc(total_len);
    memset(bufp, 0, total_len);

    recvLen = evbuffer_remove(in, bufp, total_len);
    assert(recvLen == total_len);

    lan_sync_header_t *header = (lan_sync_header_t *)bufp;
    LanSyncPkt pkt(header);

    string uri = pkt.queryXheader(XHEADER_URI);
    if (uri == "")
    {
        LOG_ERROR("[SYNC CLI] handleLanSyncReplyResource() query header is failed! ");
        free(bufp);
        return;
    }

    // 写
    string pathstr = sync_cli->rm.getRsHome() + uri;

    auto path = filesystem::path(pathstr);
    auto ppath = filesystem::absolute(path).parent_path();
    if (!filesystem::exists(ppath))
    {
        filesystem::create_directories(path.parent_path());
    }

    uint64_t ret = writeFile(pathstr, pkt);
    if (ret < 0)
    {
        free(bufp);
        return;
    }

    string content_range_str = pkt.queryXheader(XHEADER_CONTENT_RANGE);
    ContentRange cr(content_range_str);
    if (cr.isLast())
    {
        if (checkHash(pkt, pathstr))
        {
            updateSyncResourceStatus(uri, SUCCESS);
            rm.refreshTable();
            delSyncResource(uri);
        }
        else
        {
            updateSyncResourceStatus(uri, FAIL);
            remove(pathstr.data());
        }
    }

    free(bufp);
}

bool SyncCli::checkHash(LanSyncPkt &pkt, string pathstr)
{
    auto p = filesystem::path(pathstr);
    if (!filesystem::exists(p))
    {
        LOG_ERROR("[HASH CHECK] not exists: [{}]", pathstr);
        return false;
    }

    string hash = pkt.queryXheader(XHEADER_HASH);
    OpensslUtil opensslUtil;
    string theFileHash = opensslUtil.mdEncodeWithSHA3_512(pathstr);

    if (hash.compare(theFileHash) != 0)
    {
        LOG_ERROR("[HASH CHECK] hash is conflict! [{}]", pathstr);
        return false;
    }
    else
    {
        LOG_INFO("[HASH CHECK] hash is valid! [{}]", pathstr);
        return true;
    }
}

void SyncCli::handleTcpMsg(struct bufferevent *bev)
{
    struct evbuffer *in = bufferevent_get_input(bev);
    struct evbuffer *out = bufferevent_get_output(bev);

    uint32_t recvLen = evbuffer_get_length(in);

    char buf[LEN_LAN_SYNC_HEADER_T + 1] = {0};
    evbuffer_copyout(in, buf, LEN_LAN_SYNC_HEADER_T);
    lan_sync_header_t *try_header = (lan_sync_header_t *)buf;

    if (try_header->type == LAN_SYNC_TYPE_REPLY_TABLE_INDEX)
        handleLanSyncReplyTableIndex(bev, try_header, recvLen);
    else if (try_header->type == LAN_SYNC_TYPE_REPLY_RESOURCE)
        handleLanSyncReplyResource(bev, try_header, recvLen);
    else
        LOG_WARN("[SYNC CLI] receive tcp pkt : the type is unsupport!");
}

void SyncCli::connPeerWithTcp(struct sockaddr_in target_addr, uint16_t peer_tcp_port)
{
    LOG_INFO("[SYNC CLI] try to connect the tcp server!");
    target_addr.sin_port = htons(peer_tcp_port);
    target_addr.sin_family = AF_INET;

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    int tcpsock = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(tcpsock, (struct sockaddr *)&target_addr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        LOG_ERROR("[SYNC CLI] ERROR: {} ", strerror(errno));
        libevent_global_shutdown();
    }

    struct bufferevent *bev = bufferevent_socket_new(base, tcpsock, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, tcp_readcb, tcp_writecb, tcp_event_cb, base);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    addTcpSession(target_addr.sin_addr.s_addr, bev);
}

void SyncCli::handleHelloAck(struct sockaddr_in target_addr, LanSyncPkt &pkt)
{
    string peer_tcp_port_str = pkt.queryXheader(XHEADER_TCPPORT);
    uint16_t peer_tcp_port = atoi(peer_tcp_port_str.data());

    uint32_t data_len = pkt.getTotalLen() - pkt.getHeaderLen();

    LOG_DEBUG("[SYNC CLI] recive [HELLO ACK], data_len:{} , peer tcp port: {}", data_len, peer_tcp_port);

    if (st == STATE_DISCOVERING)
    {
        st = STATE_SYNC_READY;
        // todo(lutar) 形成 RESOURCE_TABLE, 这里是不是也可以引入状态机
        connPeerWithTcp(target_addr, peer_tcp_port);
    }
    else
    {
        if (!existTcpSessionByAddr(target_addr.sin_addr.s_addr))
        {
            connPeerWithTcp(target_addr, peer_tcp_port);
        }
    }
}

void SyncCli::handleUdpMsg(struct sockaddr_in target_addr, char *data, uint32_t data_len)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;
    LanSyncPkt pkt(header);
    data_len = pkt.getTotalLen() - pkt.getHeaderLen();

    if (pkt.getType() == LAN_SYNC_TYPE_HELLO_ACK)
        handleHelloAck(target_addr, pkt);
    else
        LOG_WARN("[SYNC CLI] recive [404]{} : {}", "unsupport type, do not reply ", header->type);
}

static void send_udp_hello(evutil_socket_t, short, void *arg)
{
    LOG_DEBUG("[SYNC CLI] send [HELLO]");

    string msg = "hello";
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_HELLO);
    struct evbuffer *buf = evbuffer_new();
    pkt.write(buf);

    udp_cli *cli = (udp_cli *)arg;
    cli->send(buf);
}

void SyncCli::handle_sync_status_pending(WantSyncResource *rs)
{
    // 查询tcp session是否存在
    if (existTcpSessionByBufevent(rs->bev))
    {
        rs->status = SYNCING;
        LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_RESOURCE);

        string range_hdr = rs->range.to_string();
        pkt.addXheader(XHEADER_URI, rs->uri);
        pkt.addXheader(XHEADER_RANGE, range_hdr);
        pkt.write(rs->bev);

        LOG_INFO("[SYNC CLI] [{}] : req uri[{}] range[{}]!", SERVICE_NAME_REQ_RESOURCE, rs->uri, range_hdr);
    }
    else
    {
        delSyncResource(rs->uri);
        LOG_INFO("[SYNC CLI] [{}] : tcp session is not exist, so remove the sync entry: uri[{}]", SERVICE_NAME_REQ_RESOURCE, rs->uri);
    }
}

void SyncCli::handle_sync_status_syncing(WantSyncResource *rs)
{
    time_t now = time(0);
    int diff = difftime(now, rs->last_update_time);
    if (diff > 250)
    {
        LOG_WARN("[SYNC CLI] [{}] : uri[{}] cost a lot of time! now reset status", SERVICE_NAME_REQ_RESOURCE, rs->uri);
        rs->status = PENDING;
        rs->last_update_time = time(0);
    }
}

void SyncCli::syncResource()
{
    if (syncTable.size() == 0)
        return;

    for (auto iter = syncTable.end() - 1; iter >= syncTable.begin(); iter--)
    {
        WantSyncResource *rs = (*iter);
        switch (rs->status)
        {
        case PENDING:
            handle_sync_status_pending(rs);
            break;
        case SYNCING:
            handle_sync_status_syncing(rs);
            break;
        case SUCCESS:
            LOG_INFO("[SYNC CLI] [{}] : uri[{}] sync success! ", SERVICE_NAME_REQ_RESOURCE, rs->uri);
            break;
        case FAIL:
            rs->status = PENDING;
            rs->last_update_time = time(0);
            // todo(lutar) 删除文件
            LOG_INFO("[SYNC CLI] [{}] : uri[{}] sync fail!  now reset status", SERVICE_NAME_REQ_RESOURCE, rs->uri);
            break;
        default:
            LOG_WARN(" [SYNC CLI] [{}] : uri[{}] sync status is unsupport", SERVICE_NAME_REQ_RESOURCE, rs->uri);
            break;
        }
    }
}

void SyncCli::reqTableIndex()
{
    for (auto i = tcpTable.begin(); i != tcpTable.end(); i++)
    {
        auto target_addr = (*i).first;
        auto bev = (*i).second;

        struct evbuffer *out = bufferevent_get_output(bev);

        LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_TABLE_INDEX);

        LOG_INFO("[SYNC CLI] send {}!", SERVICE_NAME_REQ_TABLE_INDEX);
        pkt.write(bev);
    }
}

void SyncCli::config_req_table_index_periodically()
{
    struct event *timeout_event = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, req_table_index_timeout_cb, nullptr);
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = PERIOD_OF_REQ_TABLE_INDEX;
    event_add(timeout_event, &tv);
}

void SyncCli::config_send_udp_periodically()
{
    vector<LocalPort> ports = LocalPort::query();

    for (size_t i = 0; i < ports.size(); i++)
    {
        LocalPort port = ports[i];

        struct sockaddr_in t_addr;
        t_addr.sin_family = AF_INET;
        t_addr.sin_port = htons(DISCOVER_SERVER_UDP_PORT);
        t_addr.sin_addr = port.getBroadAddr().sin_addr;

        udp_cli *cli = new udp_cli(udp_sock, base, t_addr);
        struct event *timeout_event = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, send_udp_hello, cli);

        struct timeval tv;
        evutil_timerclear(&tv);
        tv.tv_sec = PERIOD_OF_SEND_UDP;
        event_add(timeout_event, &tv);
        // todo (lutar) free event
    }
}

void SyncCli::config_req_resource_periodically()
{
    struct event *timeout_event = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, req_resource_periodically_timeout, nullptr);

    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = PERIOD_OF_REQ_RS;
    event_add(timeout_event, &tv);

    // todo (lutar) free event
}

void SyncCli::addSyncResource(struct WantSyncResource *item)
{
    for (size_t i = 0; i < syncTable.size(); i++)
    {
        auto rs = syncTable[i];

        if (compareChar(rs->uri, item->uri, NAME_MAX_SIZE))
        {
            free(item);
            return;
        }
    }
    syncTable.push_back(item);
}

void SyncCli::delSyncResource(string uri)
{
    for (auto i = syncTable.begin(); i != syncTable.end(); i++)
    {
        auto rs = *i;
        if (compareChar(rs->uri, uri.data(), uri.size()))
        {
            syncTable.erase(i);
            free(rs);
            return;
        }
    }
}

void SyncCli::updateSyncResourceStatus(string uri, enum WantSyncResourceStatusEnum status)
{
    for (auto iter = syncTable.end() - 1; iter >= syncTable.begin(); iter--)
    {
        WantSyncResource *rs = (*iter);
        if (compareChar(rs->uri, uri.data(), uri.size()))
        {
            rs->status = status;
            break;
        }
    }
}

void SyncCli::config_udp_sock()
{
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(udp_sock > 0);

    int optval = 1; // 这个值一定要设置，否则可能导致sendto()失败
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
}

void SyncCli::init()
{
    config_udp_sock();

    config_send_udp_periodically();

    config_req_table_index_periodically();

    config_req_resource_periodically();
}

void SyncCli::start()
{
    init();

    struct event *read_event = event_new(base, udp_sock, EV_READ | EV_PERSIST, udp_readcb, base);
    assert(read_event);
    event_add(read_event, nullptr);

    event_base_dispatch(base);
    event_free(read_event);
}

#ifndef RELEASE
int main(int argc, char const *argv[])
{
    configlog();
    sync_cli->start();
    event_base_free(base);
    free(sync_cli);
    return 0;
}
#endif