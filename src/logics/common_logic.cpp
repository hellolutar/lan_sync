#include "common_logic.h"

void SyncCommonLoigc::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    throw "SyncCommonLoigc::recv() not support recv!";
}

uint64_t SyncCommonLoigc::isExtraAllDataNow(void *data, uint64_t data_len)
{
    if (data_len < LEN_LAN_SYNC_HEADER_T)
        return 0;

    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (header->version != LAN_SYNC_VER_0_1)
    {
        LOG_WARN("SyncCommonLoigc::isExtraAllDataNow() : the protocol is unsupport! : {}", header->version);
        return 0;
    }

    uint64_t hd_total_len = ntohl(header->total_len);
    // LOG_DEBUG("SyncCommonLoigc::isExtraAllDataNow() : data_len{} \t hd_total_len:{}", data_len, hd_total_len);
    if (data_len < hd_total_len)
        return 0;
    else
        return hd_total_len;
}