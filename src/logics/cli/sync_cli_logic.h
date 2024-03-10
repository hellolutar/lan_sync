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
    NetTrigger *req_table_index_tr;
    NetTrigger *req_rs_tr;
    SyncCliDiscoverLogic discover_logic;
    SyncCliReqTbIdxLogic req_tb_idx_logic;
    SyncCliReqRsLogic req_rs;

    void handleLanSyncReplyTableIndex(void *data, uint64_t data_len, NetworkConnCtx *ctx, lan_sync_header_t *header);
    void handleLanSyncReplyResource(void *data, uint64_t data_len, NetworkConnCtx *ctx, lan_sync_header_t *header);

public:
    enum state st;
    SyncCliLogic();
    ~SyncCliLogic(){};
    void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    bool isExtraAllDataNow(void *data, uint64_t data_len) override;

    void setDiscoveryTrigger(NetTrigger *tr);
    NetTrigger &getDiscoveryTrigger();

    void setReqTbIdxTrigger(NetTrigger *tr);
    NetTrigger &getReqTableIndexTrigger();

    void setReqRsLogicTrigger(NetTrigger *tr);
    NetTrigger &getSyncCliReqRsLogicTrigger();

    SyncCliDiscoverLogic &getDiscoverLogic();
    SyncCliReqTbIdxLogic &getReqTbIdxLogic();
    SyncCliReqRsLogic &getReqRsLogic();
};

#endif