#include "sync_server.h"

SyncUdpServer::~SyncUdpServer()
{
}

void SyncUdpServer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    recv_logic->recv(data, data_len, ctx);
}

bool SyncUdpServer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return true;
}

void SyncUdpServer::setLogic(LogicUdp *recv_logic)
{
    this->recv_logic = recv_logic;
}

SyncTcpServer::~SyncTcpServer()
{
}

void SyncTcpServer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    recv_logic->recv(data, data_len, ctx);
}
bool SyncTcpServer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    if (data_len < LEN_LAN_SYNC_HEADER_T)
        return false;

    lan_sync_header_t *try_header = (lan_sync_header_t *)data;
    uint16_t total_len = ntohl(try_header->total_len);
    if (data_len < total_len)
        return false;

    return true;
}

void SyncTcpServer::setLogic(LogicTcp *recv_logic)
{
    this->recv_logic = recv_logic;
}

SyncLogic::SyncLogic(NetAbility *udpserver, NetAbility *tcpserver) : udpserver(udpserver), tcpserver(tcpserver)
{
    st = STATE_DISCOVERING;
}

SyncLogic::~SyncLogic()
{
}

bool SyncLogic::isExtraAllDataNow(void *data, uint64_t data_len)
{
    if (data_len < LEN_LAN_SYNC_HEADER_T)
        return false;

    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (data_len < ntohl(header->total_len))
        return false;

    return true;
}

void SyncLogic::recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (header->type == LAN_SYNC_TYPE_HELLO)
    {

#ifdef RELEASE
        auto ports = LocalPort::query();
        if (LocalPort::existIp(ports, target_addr.sin_addr))
            return;
#endif

        LOG_DEBUG("[SYNC SER] receive pkt : {}", SERVICE_NAME_DISCOVER_HELLO);
        // 启动TCP Server
        st = STATE_SYNC_READY;

        LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_HELLO_ACK);
        pkt.addXheader(XHEADER_TCPPORT, to_string(DISCOVER_SERVER_TCP_PORT));

        struct evbuffer *buf = evbuffer_new();
        pkt.write(buf);

        uint64_t reply_data_len = evbuffer_get_length(buf);
        uint8_t *reply_data = (uint8_t *)malloc(reply_data_len);
        evbuffer_remove(buf, reply_data, reply_data_len);
        evbuffer_free(buf);
        ctx->write(reply_data, reply_data_len);
        free(reply_data);
    }
    else
        LOG_WARN("[SYNC SER] receive pkt[{}] : the type is unsupport : {}", ctx->getNetworkEndpoint()->getAddr().str().data(), header->type);
}

void SyncLogic::recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;

    // should  use strategy pattern
    if (header->type == LAN_SYNC_TYPE_GET_TABLE_INDEX)
    {
        LOG_INFO("[SYNC SER] receive pkt: {}", SERVICE_NAME_REQ_TABLE_INDEX);
        replyTableIndex(ctx);
    }
    else if (header->type == LAN_SYNC_TYPE_GET_RESOURCE)
    {
        LOG_INFO("[SYNC SER] receive pkt: {}", SERVICE_NAME_REQ_RESOURCE);
        replyResource(header, ctx);
    }
    else
        LOG_INFO("[SYNC SER] : receive pkt: the type is unsupport!");
}

void SyncLogic::replyTableIndex(NetworkConnCtx *ctx)
{
    vector<struct Resource *> table = rm.getTable();
    uint32_t table_len = sizeof(struct Resource) * table.size();

    struct Resource *reply_table = Resource::vecToArr(table);

    struct evbuffer *buf = evbuffer_new();

    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    pkt.setData(reply_table, table_len);
    pkt.write(buf);
    free(reply_table);

    uint64_t reply_data_len = evbuffer_get_length(buf);
    uint8_t *reply_data = (uint8_t *)malloc(reply_data_len);
    evbuffer_remove(buf, reply_data, reply_data_len);
    evbuffer_free(buf);
    ctx->write(reply_data, reply_data_len);
    free(reply_data);

    LOG_INFO("[SYNC SER] [{}] : entry num: {} ", SERVICE_NAME_REPLY_TABLE_INDEX, table.size());
}

void SyncLogic::replyResource(lan_sync_header_t *header, NetworkConnCtx *ctx)
{
    LanSyncPkt pkt(header);

    string xhd_uri = pkt.queryXheader(XHEADER_URI);
    string range_str = pkt.queryXheader(XHEADER_RANGE);
    Range range(range_str);

    char *uri = xhd_uri.data();
    LOG_INFO("[SYNC SER] [{}] : uri[{}] ", SERVICE_NAME_REQ_RESOURCE, uri);

    const struct Resource *rs = rm.queryByUri(uri);
    if (rs == nullptr)
        return;

    IoReadMonitor *monitor = new SyncIOReadMonitor2(ctx, rs); // reply msg in there

    IoUtil io;
    io.addReadMonitor(monitor);

    uint64_t ret_len = 0;
    void *data = io.readAll(rs->path, ret_len);
    free(data);

    LOG_DEBUG("[SYNC SER] [{}] : uri[{}] file size:{} ", SERVICE_NAME_REPLY_REQ_RESOURCE, uri, ret_len);
}

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(base);

    struct sockaddr_in udpsock;
    udpsock.sin_family = AF_INET;
    udpsock.sin_port = htons(DISCOVER_SERVER_UDP_PORT);
    udpsock.sin_addr.s_addr = htonl(INADDR_ANY);
    auto udpserver = new SyncUdpServer(udpsock);
    NetFrameworkImplWithEvent::addUdpServer(udpserver);

    struct sockaddr_in tcpsock;
    tcpsock.sin_family = AF_INET;
    tcpsock.sin_port = htons(DISCOVER_SERVER_TCP_PORT);
    tcpsock.sin_addr.s_addr = htonl(INADDR_ANY);
    auto tcpserver = new SyncTcpServer(tcpsock);
    NetFrameworkImplWithEvent::addTcpServer(tcpserver);

    SyncLogic *recv_logic = new SyncLogic(udpserver, tcpserver);
    LogicTcp *tcplogic = recv_logic;
    LogicUdp *udplogic = recv_logic;

    udpserver->setLogic(udplogic);
    tcpserver->setLogic(tcplogic);

    NetFrameworkImplWithEvent::run();

    delete tcpserver;
    delete udpserver;
    delete recv_logic;
    NetFrameworkImplWithEvent::free();

    return 0;
}