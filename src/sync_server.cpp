#include "sync_server.h"

SyncUdpServer::~SyncUdpServer()
{
}

void SyncUdpServer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    logic->recv(data, data_len, ctx);
}

bool SyncUdpServer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return true;
}

void SyncUdpServer::setLogic(LogicUdp *logic)
{
    this->logic = logic;
}

SyncTcpServer::~SyncTcpServer()
{
}

void SyncTcpServer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    logic->recv(data, data_len, ctx);
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

void SyncTcpServer::setLogic(LogicTcp *logic)
{
    this->logic = logic;
}

SyncLogic::SyncLogic(NetworkEndpoint *udpserver, NetworkEndpoint *tcpserver) : udpserver(udpserver), tcpserver(tcpserver)
{
    st = STATE_DISCOVERING;
}

SyncLogic::~SyncLogic()
{
}

void SyncLogic::recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;

    auto target_addr = ctx->getNetworkEndpoint()->getAddr();
    target_addr->sin_addr.s_addr = ntohl(target_addr->sin_addr.s_addr);
    target_addr->sin_port = ntohs(target_addr->sin_port);

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
        LOG_WARN("[SYNC SER] receive pkt[{}:{}] : the type is unsupport : {}",
                 inet_ntoa(target_addr->sin_addr), ntohs(target_addr->sin_port), header->type);
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

    struct Resource *reply_table = resource_convert_vec_to_arr(table);

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
    struct sockaddr_in udpsock;
    udpsock.sin_family = AF_INET;
    udpsock.sin_port = htons(DISCOVER_SERVER_UDP_PORT);
    udpsock.sin_addr.s_addr = htonl(INADDR_ANY);
    auto udpserver = new SyncUdpServer(&udpsock);
    NetworkLayerWithEvent::addUdpServer(udpserver);

    struct sockaddr_in tcpsock;
    tcpsock.sin_family = AF_INET;
    tcpsock.sin_port = htons(DISCOVER_SERVER_TCP_PORT);
    tcpsock.sin_addr.s_addr = htonl(INADDR_ANY);
    auto tcpserver = new SyncTcpServer(&tcpsock);
    NetworkLayerWithEvent::addTcpServer(tcpserver);

    SyncLogic *logic = new SyncLogic(udpserver, tcpserver);
    LogicTcp *tcplogic = logic;
    LogicUdp *udplogic = logic;

    udpserver->setLogic(udplogic);
    tcpserver->setLogic(tcplogic);

    NetworkLayerWithEvent::run();

    delete tcpserver;
    delete udpserver;
    delete logic;
    NetworkLayerWithEvent::free();

    return 0;
}