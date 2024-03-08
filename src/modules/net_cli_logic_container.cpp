#include "net_cli_logic_container.h"

void NetCliLogicContainer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    logic.recv(data, data_len, ctx);
}
bool NetCliLogicContainer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return logic.isExtraAllDataNow(data, data_len);
}

void NetCliLogicContainer::setCtx(NetworkConnCtx *ctx)
{
    this->net_ctx = ctx;
}

NetworkConnCtx &NetCliLogicContainer::getConnCtx()
{
    return *net_ctx;
}
