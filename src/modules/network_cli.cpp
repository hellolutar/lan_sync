#include "network_cli.h"

void NetworkCli::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    logic.recv(data, data_len, ctx);
}
bool NetworkCli::isExtraAllDataNow(void *data, uint64_t data_len)
{
    if (data_len < LEN_LAN_SYNC_HEADER_T)
        return false;

    lan_sync_header_t *header = (lan_sync_header_t *)data;

    if (data_len < ntohl(header->total_len))
        return false;

    return true;
}

void NetworkCli::setCtx(NetworkConnCtx *ctx)
{
    this->net_ctx = ctx;
}

NetworkConnCtx &NetworkCli::getConnCtx()
{
    return *net_ctx;
}
