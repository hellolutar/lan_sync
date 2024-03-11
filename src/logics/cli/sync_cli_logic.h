#ifndef __SYNC_CLI_LOGIC_H_
#define __SYNC_CLI_LOGIC_H_

#include "vo/want_to_sync_vo.h"
#include "modules/resource_manager.h"
#include "sync_cli_sync_logic.h"
#include "sync_cli_discover_logic.h"
#include "modules/net_trigger.h"
#include "proto/lan_share_protocol.h"

class SyncCliLogic : public LogicTcp, public LogicUdp
{
private:
    void handleHelloAck(LanSyncPkt &pkt, NetworkConnCtx &ctx);

    NetTrigger *discovery_tr;
    NetTrigger *sync_tr;
    SyncCliDiscoverLogic discover_logic;
    SyncCliSyncLogic sync_logic;

    void handleLanSyncReplyTableIndex(void *data, uint64_t data_len, NetworkConnCtx *ctx, lan_sync_header_t *header);
    void handleLanSyncReplyResource(void *data, uint64_t data_len, NetworkConnCtx *ctx, lan_sync_header_t *header);

public:
    enum state st;
    SyncCliLogic();
    ~SyncCliLogic(){};
    void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len) override;

    void setDiscoveryTrigger(NetTrigger *tr);
    NetTrigger &getDiscoveryTrigger();

    void setSyncTrigger(NetTrigger *tr);
    NetTrigger &getsSyncTrigger();


    SyncCliDiscoverLogic &getDiscoverLogic();
    SyncCliSyncLogic &getSyncLogic();
};

#endif