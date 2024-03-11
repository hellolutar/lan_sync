#include "net_logic_container.h"

void NetCliLogicContainer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    recv_logic.recv(data, data_len, ctx);
}
void NetCliLogicContainer::isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len)
{
    recv_logic.isExtraAllDataNow(data, data_len, want_to_extra_len);
}

void NetCliLogicContainer::setCtx(NetworkConnCtx *ctx)
{
    this->net_ctx = ctx;
}

NetworkConnCtx &NetCliLogicContainer::getConnCtx()
{
    return *net_ctx;
}

void NetSrvLogicContainer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    recv_logic.recv(data, data_len, ctx);
};
void NetSrvLogicContainer::isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len)
{
    recv_logic.isExtraAllDataNow(data, data_len, want_to_extra_len);
};