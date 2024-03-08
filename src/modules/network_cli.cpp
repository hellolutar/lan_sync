#include "network_cli.h"

void NetCliLogicContainer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    logic.recv(data, data_len, ctx);
}
bool NetCliLogicContainer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    if (data_len < LEN_LAN_SYNC_HEADER_T)
        return false;

    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (data_len < ntohl(header->total_len))
        return false;

    return true;
}

void NetCliLogicContainer::setCtx(NetworkConnCtx *ctx)
{
    this->net_ctx = ctx;
}

NetworkConnCtx &NetCliLogicContainer::getConnCtx()
{
    return *net_ctx;
}
