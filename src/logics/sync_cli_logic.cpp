#include "sync_cli_logic.h"

bool SyncCliLogicTcp::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return recv_logic.isExtraAllDataNow(data, data_len);
}
void SyncCliLogicTcp::recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    recv_logic.recv_tcp(data, data_len, ctx);
}

void SyncCliLogicUdp::recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    recv_logic.recv_udp(data, data_len, ctx);
}

bool SyncCliLogicUdp::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return recv_logic.isExtraAllDataNow(data, data_len);
}

SyncCliLogic::SyncCliLogic()
{
}

void SyncCliLogic::setDiscoveryTrigger(NetTrigger *tr)
{
    discovery = tr;
}
void SyncCliLogic::setReqTableIndexTrigger(NetTrigger *tr)
{
    req_table_index = tr;
}

SyncCliDiscoverLogic &SyncCliLogic::getDiscoverLogic()
{
    return discover_logic;
}

SyncCliReqTbIdxLogic &SyncCliLogic::getSyncCliReqTbIdxLogic()
{
    return req_tb_idx_logic;
}

NetTrigger &SyncCliLogic::getDiscoveryTrigger()
{
    return *discovery;
}

NetTrigger &SyncCliLogic::getReqTableIndexTrigger()
{
    return *req_table_index;
}

bool SyncCliLogic::isExtraAllDataNow(void *data, uint64_t data_len)
{
    if (data_len < LEN_LAN_SYNC_HEADER_T)
        return false;

    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (data_len < ntohl(header->total_len))
        return false;

    return true;
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
    this->req_table_index->addNetAddr(peer_tcp_addr);
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
    ResourceManager::getRsm().analysisThenUpdateSyncTable(table, res_size);
}

void SyncCliLogic::handleLanSyncReplyResource(void *data, uint64_t data_len, NetworkConnCtx *ctx, lan_sync_header_t *header)
{
    ResourceManager &rsm = ResourceManager::getRsm();

    LanSyncPkt pkt(header);

    string uri = pkt.queryXheader(XHEADER_URI);
    if (uri == "")
    {
        LOG_ERROR("[SYNC CLI] handleLanSyncReplyResource() query header is failed! ");
        return;
    }

    string content_range_str = pkt.queryXheader(XHEADER_CONTENT_RANGE);
    ContentRange cr(content_range_str);

    uint8_t *data_pos = (uint8_t *)data + pkt.getHeaderLen();
    bool write_ret = rsm.saveLocal(uri, data_pos, cr.getStartPos(), cr.getSize());
    if (!write_ret)
    {
        rsm.updateSyncEntryStatus(uri, FAIL);
        return;
    }

    if (cr.isLast())
    {
        string hash = pkt.queryXheader(XHEADER_HASH);
        rsm.validRes(uri, hash);
    }
}