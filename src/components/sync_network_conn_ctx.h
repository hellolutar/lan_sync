#ifndef __SYNC_NETWORK_CONN_CTX_H_
#define __SYNC_NETWORK_CONN_CTX_H_

#include <string>

#include "net/network_layer.h"
#include "proto/lan_share_protocol.h"

class SyncNetworkConnCtx : public NetworkConnCtx
{
private:
    LanSyncPkt &pkt;
    std::shared_ptr<NetworkConnCtx> nctx;

public:
    SyncNetworkConnCtx(std::shared_ptr<NetworkConnCtx> nctx, LanSyncPkt &pkt) : nctx(nctx), pkt(pkt){};
    ~SyncNetworkConnCtx();

    uint64_t write(void *data, uint64_t data_len) override;
    NetAbility *getNetworkEndpoint() override;
    NetAddr &getPeer() override;
    void setNetAddr(NetAddr peer) override;
    bool isActive() override;
    void setActive(bool status) override;

    std::string queryXheader(std::string key);
    LanSyncPkt &getPktInfo();
};

#endif