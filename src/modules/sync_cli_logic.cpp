#include "sync_cli_logic.h"

SyncCliLogic::~SyncCliLogic()
{
}

void SyncCliLogic::setDiscoveryTrigger(NetTrigger *tr)
{
    discovery = tr;
}

NetTrigger &SyncCliLogic::getDiscoveryTrigger()
{
    return *discovery;
}

void SyncCliLogic::handleHelloAck(LanSyncPkt &pkt)
{
    string peer_tcp_port_str = pkt.queryXheader(XHEADER_TCPPORT);
    uint16_t peer_tcp_port = atoi(peer_tcp_port_str.data());

    uint32_t data_len = pkt.getTotalLen() - pkt.getHeaderLen();

    LOG_DEBUG("[SYNC CLI] recive [HELLO ACK], data_len:{} , peer tcp port: {}", data_len, peer_tcp_port);

    if (st == STATE_DISCOVERING)
    {
        st = STATE_SYNC_READY;
        // todo(lutar) 形成 RESOURCE_TABLE, 这里是不是也可以引入状态机
        // connPeerWithTcp(target_addr, peer_tcp_port);
        // 向trigger中添加连接
    }
    // else
    // {
    //     if (!existTcpSessionByAddr(target_addr.sin_addr.s_addr))
    //     {
    //         connPeerWithTcp(target_addr, peer_tcp_port);
    //     }
    // }

    // todo add tcp cli trigger
}

void SyncCliLogic::recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;
    LanSyncPkt pkt(header);

    if (pkt.getType() == LAN_SYNC_TYPE_HELLO_ACK)
        handleHelloAck(pkt);
    else
        LOG_WARN("[SYNC CLI] recive [404]{} : {}", "unsupport type, do not reply ", header->type);
}

void SyncCliLogic::recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
}

SyncCliLogicTcp::~SyncCliLogicTcp()
{
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