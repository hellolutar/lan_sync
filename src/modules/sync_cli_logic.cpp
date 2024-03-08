#include "sync_cli_logic.h"

SyncCliLogic::~SyncCliLogic()
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

NetTrigger &SyncCliLogic::getDiscoveryTrigger()
{
    return *discovery;
}

NetTrigger &SyncCliLogic::ReqTableIndexTrigger()
{
    return *req_table_index;
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

SyncCliLogicTcp::~SyncCliLogicTcp()
{
}

bool SyncCliLogicTcp::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return logic.isExtraAllDataNow(data, data_len);
}
void SyncCliLogicTcp::recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    logic.recv_tcp(data, data_len, ctx);
}

SyncCliLogicUdp::~SyncCliLogicUdp()
{
}

void SyncCliLogicUdp::recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    logic.recv_udp(data, data_len, ctx);
}

bool SyncCliLogicUdp::isExtraAllDataNow(void *data, uint64_t data_len)
{
   return logic.isExtraAllDataNow(data, data_len);
}