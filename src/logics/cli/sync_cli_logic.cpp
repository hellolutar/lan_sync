#include "sync_cli_logic.h"

SyncCliLogic::SyncCliLogic()
{
}

void SyncCliLogic::setDiscoveryTrigger(NetTrigger *tr)
{
    discovery_tr = tr;
}
void SyncCliLogic::setSyncTrigger(NetTrigger *tr)
{
    sync_tr = tr;
}

SyncCliDiscoverLogic &SyncCliLogic::getDiscoverLogic()
{
    return discover_logic;
}

SyncCliSyncLogic &SyncCliLogic::getSyncLogic()
{
    return sync_logic;
}

NetTrigger &SyncCliLogic::getDiscoveryTrigger()
{
    return *discovery_tr;
}

NetTrigger &SyncCliLogic::getsSyncTrigger()
{
    return *sync_tr;
}

void SyncCliLogic::isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len)
{
    if (data_len < LEN_LAN_SYNC_HEADER_T)
        want_to_extra_len = 0;

    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (header->version != LAN_SYNC_VER_0_1)
    {
        LOG_WARN("SyncCliLogic::isExtraAllDataNow() : the protocol is unsupport! : {}", header->version);
        want_to_extra_len = 0;
    }

    uint64_t hd_total_len = ntohl(header->total_len);
    if (data_len < hd_total_len)
        want_to_extra_len = 0;
    else if (data_len > hd_total_len)
        want_to_extra_len = hd_total_len;
    else
        want_to_extra_len = data_len;

}

void SyncCliLogic::handleHelloAck(LanSyncPkt &pkt, NetworkConnCtx &ctx)
{
    string peer_tcp_port_str = pkt.queryXheader(XHEADER_TCPPORT);
    uint16_t peer_tcp_port = atoi(peer_tcp_port_str.data());

    uint32_t data_len = pkt.getTotalLen() - pkt.getHeaderLen();

    LOG_DEBUG("[SYNC CLI] recive [HELLO ACK], data_len:{} , peer tcp port: {}", data_len, peer_tcp_port);

    if (st == STATE_DISCOVERING)
        st = STATE_SYNC_READY;

    NetAddr peer_tcp_addr = ctx.getPeer();
    peer_tcp_addr.setPort(peer_tcp_port);
    this->sync_tr->addConn(peer_tcp_addr);
}

void SyncCliLogic::recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;
    LanSyncPkt pkt(header);

    if (pkt.getType() == LAN_SYNC_TYPE_HELLO_ACK)
        handleHelloAck(pkt, *ctx);
    else
        LOG_WARN("[SYNC CLI] recive [404]{} : {}", "unsupport type, do not reply ", header->type);
}

void SyncCliLogic::recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (header->type == LAN_SYNC_TYPE_REPLY_TABLE_INDEX)
        handleLanSyncReplyTableIndex(data, data_len, ctx, header);
    else if (header->type == LAN_SYNC_TYPE_REPLY_RESOURCE)
        handleLanSyncReplyResource(data, data_len, ctx, header);
    else
        LOG_WARN("[SYNC CLI] receive tcp pkt : the type is unsupport!");
}

void SyncCliLogic::handleLanSyncReplyTableIndex(void *data, uint64_t data_len, NetworkConnCtx *ctx, lan_sync_header_t *header)
{
    LanSyncPkt pkt(header);

    uint64_t res_size = pkt.getDataLen() / sizeof(struct Resource);

    struct Resource *table = (struct Resource *)pkt.getData();
    // ResourceManager::getRsLocalManager().analysisThenUpdateSyncTable(ctx->peer, table, res_size);
    // TODO
}

void SyncCliLogic::handleLanSyncReplyResource(void *data, uint64_t data_len, NetworkConnCtx *ctx, lan_sync_header_t *header)
{
    // TODO
    // RsLocalManager &rsm = ResourceManager::getRsLocalManager();

    // LanSyncPkt pkt(header);

    // string uri = pkt.queryXheader(XHEADER_URI);
    // if (uri == "")
    // {
    //     LOG_ERROR("[SYNC CLI] handleLanSyncReplyResource() query header is failed! ");
    //     return;
    // }

    // string content_range_str = pkt.queryXheader(XHEADER_CONTENT_RANGE);
    // ContentRange cr(content_range_str);
    // LOG_INFO("[SYNC CLI] SyncCliLogic::handleLanSyncReplyResource() : uri:{}; cr:{}", uri.data(), cr.to_string());

    // uint8_t *data_pos = (uint8_t *)data + pkt.getHeaderLen();
    // bool write_ret = rsm.saveLocal(uri, data_pos, cr.getStartPos(), cr.getSize());
    // if (!write_ret)
    // {
    //     rsm.updateSyncEntryStatus(uri, FAIL);
    //     return;
    // }

    // if (cr.isLast())
    // {
    //     string hash = pkt.queryXheader(XHEADER_HASH);
    //     rsm.validRes(uri, hash);
    // }
}