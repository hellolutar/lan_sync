#include "sync_service.h"

SyncService::~SyncService()
{
}

void SyncService::mod_conn_recv(std::string from, std::string uri, void *data)
{
    LOG_DEBUG("SyncService::mod_conn_recv() : from:{},\turi:{}", from, uri);
    // TODO
}

int SyncService::queryPeerTcpServerPort(LanSyncPkt &pkt)
{
    string peer_tcp_port_str = pkt.queryXheader(XHEADER_TCP_SRV_PORT);
    if (peer_tcp_port_str.size() == 0)
    {
        LOG_WARN("SyncService::handleHelloAck() : tcp port is not found in reply header!");
        return 0;
    }
    uint16_t peer_tcp_port = atoi(peer_tcp_port_str.data());
    return peer_tcp_port;
}

void SyncService::handleHello(SyncNetworkConnCtx *ctx)
{
    // 启动TCP Server
    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_HELLO_ACK);
    pkt.addXheader(XHEADER_TCP_SRV_PORT, ConfigManager::query(CONFIG_KEY_PROTO_SYNC_SERVER_TCP_PORT));

    BufBaseonEvent buf;
    pkt.write(buf);
    ctx->write(buf.data(), buf.size());

    int peerTcpServerPort = queryPeerTcpServerPort(pkt);
    if (peerTcpServerPort > 1024)
    {
        NetAddr peer_tcp_addr = ctx->getPeer();
        peer_tcp_addr.setPort(peerTcpServerPort);
        mod_conn_send(MODULE_NAME_PERIOD_REQ_TB_IDX, MODULE_CONN_URI_PERIOD_REQ_TB_IDX_ADD, &peer_tcp_addr);
    }
}

void SyncService::handleHelloAck(SyncNetworkConnCtx *ctx)
{
    LanSyncPkt &pkt = ctx->getPktInfo();

    int peerTcpServerPort = queryPeerTcpServerPort(pkt);
    NetAddr peer_tcp_addr = ctx->getPeer();
    peer_tcp_addr.setPort(peerTcpServerPort);
    LOG_DEBUG("SyncService::handleHelloAck() : receive hello ack, try to connect peer with tcp:{}", peer_tcp_addr.str());

    mod_conn_send(MODULE_NAME_PERIOD_REQ_TB_IDX, MODULE_CONN_URI_PERIOD_REQ_TB_IDX_ADD, &peer_tcp_addr);
}
void SyncService::getSyncGui(SyncNetworkConnCtx *ctx)
{
    LanSyncPkt &pkt = ctx->getPktInfo();
}

void SyncService::handleReqTableIndex(SyncNetworkConnCtx *ctx)
{
    RsLocalManager &rlm = ResourceManager::getRsLocalManager();
    vector<struct Resource *> table = rlm.getTable();
    uint32_t table_len = sizeof(struct Resource) * table.size();

    struct Resource *reply_table = Resource::vecToArr(table);

    LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    pkt.setData(reply_table, table_len);

    BufBaseonEvent buf;
    pkt.write(buf);
    free(reply_table);

    ctx->write(buf.data(), buf.size());

    LOG_INFO("SyncService::handleReqTableIndex() : entry num: {} ", table.size());
}

void SyncService::handleReqResource(SyncNetworkConnCtx *ctx)
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

void SyncService::handleReplyTableIndex(void *data, uint64_t data_len, SyncNetworkConnCtx *ctx)
{
    LanSyncPkt &pkt = ctx->getPktInfo();

    uint64_t res_size = pkt.getDataLen() / sizeof(struct Resource);

    struct Resource *table = (struct Resource *)pkt.getData();
    RsSyncManager &rsm = ResourceManager::getRsSyncManager();
    rsm.refreshSyncingRsByTbIdx(ctx->getPeer(), table, res_size);

    // req rs
    add_req_task(ctx);
}

void SyncService::add_req_task(SyncNetworkConnCtx *ctx)
{
    RsSyncManager &rsm = ResourceManager::getRsSyncManager();

    for (auto uriRs : rsm.getAllUriRs())
    {
        string uri = uriRs.first;
        if (rsm.getSyncingSize(uri) < DOWNLOAD_LIMIT)
        {
            LOG_INFO("SyncService::add_req_task() : add ReqRsTask : {}", uri.data());
            TaskManager::getTaskManager()->addTask(new ReqRsTask(uri, uri));
        }
        else
        {
            LOG_INFO("SyncService::add_req_task() : [{}] download limit : {}", uri.data(), rsm.getSyncingSize(uri));
        }
    }
}

void SyncService::handleReplyResource(void *data, uint64_t data_len, SyncNetworkConnCtx *ctx)
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

    ResourceManager::save(ctx->getPeer(), uri, data_pos, cr.getStartPos(), cr.getSize());

    add_req_task(ctx);
}

void SyncService::exit(void *data, uint64_t data_len, SyncNetworkConnCtx *ctx)
{
    TaskManager::getTaskManager()->stop();
    NetFrameworkImplWithEvent::shutdown();
}