#ifndef __SYNC_CLI_LOGIC_H_
#define __SYNC_CLI_LOGIC_H_

#include "proto/lan_share_protocol.h"
#include "utils/logger.h"
#include "modules/abst_net_logic.h"
#include "modules/net_trigger.h"
#include "sync_cli_discover_logic.h"
#include "sync_cli_sync_logic.h"
#include "modules/resource_manager.h"
#include "vo/want_to_sync_vo.h"

class SyncCliLogic : public LogicTcp, public LogicUdp
{
private:
    void handleHelloAck(LanSyncPkt &pkt, NetworkConnCtx &ctx);

    NetTrigger *discovery;
    NetTrigger *req_table_index;
    SyncCliDiscoverLogic discover_logic;
    SyncCliReqTbIdxLogic req_tb_idx_logic;

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

    void setReqTableIndexTrigger(NetTrigger *tr);
    NetTrigger &getReqTableIndexTrigger();

    SyncCliDiscoverLogic &getDiscoverLogic();
    SyncCliReqTbIdxLogic &getSyncCliReqTbIdxLogic();
};

class SyncCliLogicTcp : public LogicTcp
{
private:
    LogicTcp &recv_logic;

public:
    SyncCliLogicTcp(LogicTcp &recv_logic) : recv_logic(recv_logic){};
    ~SyncCliLogicTcp(){};
    void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    bool isExtraAllDataNow(void *data, uint64_t data_len) override;
};

class SyncCliLogicUdp : public LogicUdp
{
private:
    LogicUdp &recv_logic;

public:
    SyncCliLogicUdp(LogicUdp &recv_logic) : recv_logic(recv_logic){};
    ~SyncCliLogicUdp(){};
    void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    bool isExtraAllDataNow(void *data, uint64_t data_len) override;
};

#endif