#include "logics/controllers/sync_controller.h"

SyncController::~SyncController()
{
}

void SyncController::recv_udp(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> nctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;
    LanSyncPkt pkt(header);
    std::shared_ptr<SyncNetworkConnCtx> ctx = make_shared<SyncNetworkConnCtx>(nctx, pkt);

#ifdef RELEASE
    auto ports = LocalPort::query();
    if (LocalPort::existIp(ports, ctx->getPeer().getBeAddr().sin_addr))
        return;
#endif

    LOG_INFO("SyncController::recv_udp() : {}, from: {}",
             convert_lan_sync_type_enum(pkt.getType()).data(), nctx->getPeer().str().data());

    if (pkt.getType() == LAN_SYNC_TYPE_HELLO)
        syncService.handleHello(ctx);
    else if (pkt.getType() == LAN_SYNC_TYPE_HELLO_ACK)
        syncService.handleHelloAck(ctx);
    else
        LOG_WARN(" SyncController::recv_udp() : {}", "unsupport type, do not reply ", static_cast<int>(header->type));
}

void SyncController::recv_tcp(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> nctx)
{
    lan_sync_header_t *header = (lan_sync_header_t *)data;
    LanSyncPkt pkt(header);
    std::shared_ptr<SyncNetworkConnCtx> ctx = make_shared<SyncNetworkConnCtx>(nctx, pkt);

#ifdef RELEASE
    auto ports = LocalPort::query();
    if (LocalPort::existIp(ports, ctx->getPeer().getBeAddr().sin_addr))
        return;
#endif

    LOG_INFO("SyncController::recv_tcp() : {}, from: {}",
             convert_lan_sync_type_enum(pkt.getType()).data(), nctx->getPeer().str().data());

    if (header->type == LAN_SYNC_TYPE_GET_TABLE_INDEX)
        syncService.handleReqTableIndex(ctx);
    else if (header->type == LAN_SYNC_TYPE_GET_RESOURCE)
        syncService.handleReqResource(ctx);
    else if (header->type == LAN_SYNC_TYPE_REPLY_TABLE_INDEX)
        syncService.handleReplyTableIndex(data, data_len, ctx);
    else if (header->type == LAN_SYNC_TYPE_REPLY_RESOURCE)
        syncService.handleReplyResource(data, data_len, ctx);
    else if (header->type == LAN_SYNC_TYPE_EXIT)
        syncService.exit(data, data_len, ctx);
    else
        LOG_WARN("SyncController::recv_tcp() : the type is unsupport!");
}

uint64_t SyncController::isExtraAllDataNow(void *data, uint64_t data_len)
{
    if (data_len < LEN_LAN_SYNC_HEADER_T)
        return 0;

    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (header->version != lan_sync_version::VER_0_1)
    {
        LOG_WARN("SyncController::isExtraAllDataNow() : the protocol is unsupport! : {}", static_cast<int>(header->version));
        return 0;
    }

    uint64_t hd_total_len = ntohl(header->total_len);
    // LOG_DEBUG("SyncCommonLoigc::isExtraAllDataNow() : data_len{} \t hd_total_len:{}", data_len, hd_total_len);
    if (data_len < hd_total_len)
        return 0;
    else
        return hd_total_len;
}