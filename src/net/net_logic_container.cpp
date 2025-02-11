#include "net_logic_container.h"

NetCliLogicContainer::~NetCliLogicContainer()
{
    if (net_ctx != nullptr)
        delete net_ctx;
}

void NetCliLogicContainer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    recv_logic.recv(data, data_len, ctx);
}
uint64_t NetCliLogicContainer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return recv_logic.isExtraAllDataNow(data, data_len);
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
uint64_t NetSrvLogicContainer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return recv_logic.isExtraAllDataNow(data, data_len);
};