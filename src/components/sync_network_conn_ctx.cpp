#include "sync_network_conn_ctx.h"

using namespace std;

string SyncNetworkConnCtx::queryXheader(string key)
{
    return pkt.queryXheader(key);
}

LanSyncPkt &SyncNetworkConnCtx::getPktInfo()
{
    return pkt;
}

SyncNetworkConnCtx::~SyncNetworkConnCtx()
{
}

uint64_t SyncNetworkConnCtx::write(void *data, uint64_t data_len)
{
    return nctx->write(data, data_len);
}

NetAbility *SyncNetworkConnCtx::getNetworkEndpoint()
{
    return nctx->getNetworkEndpoint();
}

NetAddr &SyncNetworkConnCtx::getPeer()
{
    return nctx->getPeer();
}

void SyncNetworkConnCtx::setNetAddr(NetAddr peer)
{
    return nctx->setNetAddr(peer);
}

bool SyncNetworkConnCtx::isActive()
{
    return nctx->isActive();
}

void SyncNetworkConnCtx::setActive(bool status)
{
    nctx->setActive(status);
}
