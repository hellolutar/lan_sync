#include "sync_srv_logic.h"

SyncSrvLogic::SyncSrvLogic()
{
    st = STATE_DISCOVERING;
}

void SyncSrvLogic::isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len)
{
    if (data_len < LEN_LAN_SYNC_HEADER_T)
        want_to_extra_len = 0;

    lan_sync_header_t *header = (lan_sync_header_t *)data;

    uint64_t hd_total_len = ntohl(header->total_len);
    if (data_len < hd_total_len)
        want_to_extra_len = 0;
    else if (data_len > hd_total_len)
        want_to_extra_len = hd_total_len;
    else
        want_to_extra_len = data_len;
}

void SyncSrvLogic::recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (header->type == LAN_SYNC_TYPE_HELLO)
    {

#ifdef RELEASE
        auto ports = LocalPort::query();
        if (LocalPort::existIp(ports, target_addr.sin_addr))
            return;
#endif

        LOG_DEBUG("[SYNC SER] UDP receive pkt : {}", SERVICE_NAME_DISCOVER_HELLO);
        // 启动TCP Server
        st = STATE_SYNC_READY;

        LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_HELLO_ACK);
        pkt.addXheader(XHEADER_TCPPORT, to_string(DEFAULT_DISCOVER_SERVER_TCP_PORT));

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
        LOG_WARN("[SYNC SER] UDP receive pkt[{}] : the type is unsupport : {}", ctx->getNetworkEndpoint()->getAddr().str().data(), header->type);
}

void SyncSrvLogic::recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;

    // should  use strategy pattern
    if (header->type == LAN_SYNC_TYPE_GET_TABLE_INDEX)
    {
        LOG_INFO("[SYNC SER] TCP receive pkt: {}", SERVICE_NAME_REQ_TABLE_INDEX);
        replyTableIndex(ctx);
    }
    else if (header->type == LAN_SYNC_TYPE_GET_RESOURCE)
    {
        LOG_INFO("[SYNC SER] TCP receive pkt: {}", SERVICE_NAME_REQ_RESOURCE);
        replyResource(header, ctx);
    }
    else
        LOG_INFO("[SYNC SER] TCP: receive pkt: the type is unsupport! : {}", header->type);
}

void SyncSrvLogic::replyTableIndex(NetworkConnCtx *ctx)
{
    // TODO
    // RsLocalManager &rsm = ResourceManager::getRsLocalManager();
    // vector<struct Resource *> table = rsm.getTable();
    // uint32_t table_len = sizeof(struct Resource) * table.size();

    // struct Resource *reply_table = Resource::vecToArr(table);

    // struct evbuffer *buf = evbuffer_new();

    // LanSyncPkt pkt(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_REPLY_TABLE_INDEX);
    // pkt.setData(reply_table, table_len);
    // pkt.write(buf);
    // free(reply_table);

    // uint64_t reply_data_len = evbuffer_get_length(buf);
    // uint8_t *reply_data = (uint8_t *)malloc(reply_data_len);
    // evbuffer_remove(buf, reply_data, reply_data_len);
    // evbuffer_free(buf);
    // ctx->write(reply_data, reply_data_len);
    // free(reply_data);

    // LOG_INFO("[SYNC SER] [{}] : entry num: {} ", SERVICE_NAME_REPLY_TABLE_INDEX, table.size());
}

void SyncSrvLogic::replyResource(lan_sync_header_t *header, NetworkConnCtx *ctx)
{
    LanSyncPkt pkt(header);

    string xhd_uri = pkt.queryXheader(XHEADER_URI);
    string range_str = pkt.queryXheader(XHEADER_RANGE);
    Range range(range_str);

    char *uri = xhd_uri.data();
    LOG_INFO("[SYNC SER] [{}] : uri[{}] ", SERVICE_NAME_REQ_RESOURCE, uri);

// TODO
    // RsLocalManager &rsm = ResourceManager::getRsLocalManager();
    // const struct Resource *rs = rsm.queryByUri(uri);
    // if (rs == nullptr)
    //     return;

    // IoReadMonitor *monitor = new SyncIOReadMonitor(ctx, rs); // reply msg in there

    // IoUtil io;
    // io.addReadMonitor(monitor);

    // uint64_t ret_len = 0;
    // void *data = io.readAll(rs->path, ret_len);
    // free(data);

    // LOG_DEBUG("[SYNC SER] [{}] : uri[{}] file size:{} ", SERVICE_NAME_REPLY_REQ_RESOURCE, uri, ret_len);
}
