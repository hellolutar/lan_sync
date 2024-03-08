#ifndef __SYNC_CLI_LOGIC_H_
#define __SYNC_CLI_LOGIC_H_

#include "proto/lan_share_protocol.h"
#include "utils/logger.h"
#include "abst_net_logic.h"
#include "net_trigger.h"

class SyncCliLogic : public LogicTcp, public LogicUdp
{
private:
    void handleHelloAck(LanSyncPkt &pkt, NetworkConnCtx &ctx);
    NetTrigger *discovery;
    NetTrigger *req_table_index;

public:
    enum state st;

    SyncCliLogic(){};
    ~SyncCliLogic();
    void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    bool isExtraAllDataNow(void *data, uint64_t data_len) override;

    void setDiscoveryTrigger(NetTrigger *tr);
    NetTrigger &getDiscoveryTrigger();

    void setReqTableIndexTrigger(NetTrigger *tr);
    NetTrigger &ReqTableIndexTrigger();
};

class SyncCliLogicTcp : public LogicTcp
{
private:
    LogicTcp &logic;

public:
    SyncCliLogicTcp(LogicTcp &logic) : logic(logic){};
    ~SyncCliLogicTcp();
    void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    bool isExtraAllDataNow(void *data, uint64_t data_len) override;
};

class SyncCliLogicUdp : public LogicUdp
{
private:
    LogicUdp &logic;

public:
    SyncCliLogicUdp(LogicUdp &logic) : logic(logic){};
    ~SyncCliLogicUdp();
    void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    bool isExtraAllDataNow(void *data, uint64_t data_len) override;
};

#endif