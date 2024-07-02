#include "logics/services/sync_service.h"

using namespace module_conn_uri;

SyncService::~SyncService()
{
}

void SyncService::mod_conn_recv(std::string from, std::string uri, void *data)
{
    LOG_DEBUG("SyncService::mod_conn_recv() : from:{},\turi:{}", from, uri);
    // TODO
}

void SyncService::handleHello(std::shared_ptr<SyncNetworkConnCtx> ctx)
{
    // 启动TCP Server
    LanSyncPkt pkt(lan_sync_version::VER_0_1, LAN_SYNC_TYPE_HELLO_ACK);
    pkt.addXheader(XHEADER_TCPPORT, ConfigManager::query(CONFIG_KEY_PROTO_SYNC_SERVER_TCP_PORT));

    BufBaseonEvent buf;
    pkt.write(buf);
    ctx->write(buf.data(), buf.size());
}

void SyncService::handleHelloAck(std::shared_ptr<SyncNetworkConnCtx> ctx)
{
    LanSyncPkt &pkt = ctx->getPktInfo();
    string peer_tcp_port_str = pkt.queryXheader(XHEADER_TCPPORT);
    if (peer_tcp_port_str.size() == 0)
    {
        LOG_WARN("SyncService::handleHelloAck() : tcp port is not found in reply header!");
        return;
    }
    uint16_t peer_tcp_port = atoi(peer_tcp_port_str.data());

    NetAddr peer_tcp_addr = ctx->getPeer();
    peer_tcp_addr.setPort(peer_tcp_port);
    LOG_DEBUG("SyncService::handleHelloAck() : receive hello ack, try to connect peer with tcp:{}", peer_tcp_addr.str());

    mod_conn_send(MODULE_NAME_PERIOD_REQ_TB_IDX, PERIOD_REQ_TB_IDX_ADD, &peer_tcp_addr);
}

void SyncService::handleReqTableIndex(std::shared_ptr<SyncNetworkConnCtx> ctx)
{
    RsLocalManager &rlm = ResourceManager::getRsLocalManager();
    vector<struct Resource *> table = rlm.getTable();
    uint32_t table_len = sizeof(struct Resource) * table.size();

    struct Resource *reply_table = Resource::vecToArr(table);

    LanSyncPkt pkt(lan_sync_version::VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    pkt.setData(reply_table, table_len);

    BufBaseonEvent buf;
    pkt.write(buf);
    free(reply_table);

    ctx->write(buf.data(), buf.size());

    LOG_INFO("SyncService::handleReqTableIndex() : entry num: {} ", table.size());
}

void SyncService::handleReqResource(std::shared_ptr<SyncNetworkConnCtx> ctx)
{
    LanSyncPkt &pkt = ctx->getPktInfo();

    string xhd_uri = pkt.queryXheader(XHEADER_URI);
    string range_str = pkt.queryXheader(XHEADER_RANGE);
    Range range(range_str);

    char *uri = xhd_uri.data();
    LOG_INFO("SyncService::handleReqResource() : uri[{}] range:{} ", uri, range.to_string());

    // TODO
    RsLocalManager &rlm = ResourceManager::getRsLocalManager();
    const struct Resource *rs = rlm.queryByUri(uri);
    if (rs == nullptr)
        return;

    IoReadMonitor *monitor = new SyncIOReadMonitor(*ctx, rs, range); // reply msg in there

    IoUtil io;
    io.addReadMonitor(monitor);

    uint64_t ret_len = 0;
    try
    {
        void *data = io.readSize(rs->path, range.getStartPos(), range.getSize(), ret_len);
        free(data);
        LOG_DEBUG("SyncService::handleReqResource()  : uri[{}] file size:{} ", uri, ret_len);
    }
    catch (const std::exception &e)
    {
        LOG_ERROR("SyncService::handleReqResource() : reason:{}", e.what());
    }
}

void SyncService::handleReplyTableIndex(void *data, uint64_t data_len, std::shared_ptr<SyncNetworkConnCtx> ctx)
{
    LanSyncPkt &pkt = ctx->getPktInfo();

    uint64_t table_size = pkt.getDataLen() / sizeof(struct Resource);

    struct Resource *tablep = (struct Resource *)pkt.getData();
    vector<Resource> table = Resource::arrToVec(tablep, table_size);

    RsLocalManager &rlm = ResourceManager::getRsLocalManager();
    std::vector<Resource> need_to_sync_table = rlm.cmpThenRetNeedToSyncTable(table);
    for (auto i = 0; i < need_to_sync_table.size(); i++)
    {
        Resource rs = need_to_sync_table[i];
        string uri(rs.uri);
        Range2 range(0, rs.size);
        coor_.add_resource(uri, range, ctx);
    }
}

void SyncService::handleReplyResource(void *data, uint64_t data_len, std::shared_ptr<SyncNetworkConnCtx> ctx)
{
    LanSyncPkt &pkt = ctx->getPktInfo();
    string uri = pkt.queryXheader(XHEADER_URI);
    if (uri == "")
    {
        LOG_ERROR("SyncService::handleLanSyncReplyResource() : query header is failed! ");
        return;
    }

    string content_range_str = pkt.queryXheader(XHEADER_CONTENT_RANGE);
    ContentRange cr(content_range_str);
    LOG_INFO("SyncService::handleLanSyncReplyResource() : uri:{}; cr:{}", uri.data(), cr.to_string());

    uint8_t *data_pos = (uint8_t *)data + pkt.getHeaderLen();
    RsLocalManager &rlm = ResourceManager::getRsLocalManager();

    bool write_ret = rlm.saveLocal(uri, data_pos, cr.getStartPos(), cr.getSize());

    Block2 b(cr.getStartPos(), cr.getStartPos() + cr.getSize());
    if (!write_ret)
        LOG_INFO("SyncService::handleLanSyncReplyResource() : {} : block save fail:[{},{})", uri, b.start, b.end);
}

void SyncService::exit(void *data, uint64_t data_len, std::shared_ptr<SyncNetworkConnCtx> ctx)
{
    mod_conn_send(MODULE_NAME_TASK_COORDINATOR_TRIGGER_MODCONN, TASK_COORDINATOR_TRIGGER_MODCONN_STOP, nullptr);
    NetFrameworkImplWithEvent::shutdown();
}