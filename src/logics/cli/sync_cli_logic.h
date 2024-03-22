#ifndef __SYNC_CLI_LOGIC_H_
#define __SYNC_CLI_LOGIC_H_

#include "vo/want_to_sync_vo.h"
#include "sync_cli_sync_logic.h"
#include "sync_cli_discover_logic.h"
#include "modules/net_trigger.h"
#include "proto/lan_share_protocol.h"
#include "modules/rs_manager.h"
#include "modules/task/req_rs_task.h"
#include "modules/task/task_manager.h"
#include "logics/common_logic.h"
#include "modules/conn/mod_conn.h"

class SyncCliLogic : public LogicTcp, public LogicUdp, public ModConnAbility
{
private:
    void handleHelloAck(LanSyncPkt &pkt, NetworkConnCtx *ctx);

    NetTrigger *discovery_tr;
    NetTrigger *sync_tr;
    SyncCliDiscoverLogic discover_logic;
    SyncCliSyncLogic sync_logic;
    SyncCommonLoigc commonLogic;

    void handleLanSyncReplyTableIndex(void *data, uint64_t data_len, NetworkConnCtx *ctx, lan_sync_header_t *header);
    void handleLanSyncReplyResource(void *data, uint64_t data_len, NetworkConnCtx *ctx, lan_sync_header_t *header);
    void add_req_task(NetworkConnCtx *ctx);

public:
    enum state st;
    SyncCliLogic(AbsModConnMediator &med, std::string name);
    ~SyncCliLogic(){};
    void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    uint64_t isExtraAllDataNow(void *data, uint64_t data_len) override;

    void mod_conn_recv(std::string from, std::string uri, void *data) override;
    void mod_conn_send(std::string to, std::string uri, void *data) override;

    void setDiscoveryTrigger(NetTrigger *tr);
    NetTrigger &getDiscoveryTrigger();

    void setSyncTrigger(NetTrigger *tr);
    NetTrigger &getsSyncTrigger();

    SyncCliDiscoverLogic &getDiscoverLogic();
    SyncCliSyncLogic &getSyncLogic();
};

#endif