#ifndef __SYNC_CLI_LOGIC_H_
#define __SYNC_CLI_LOGIC_H_

#include "logic.h"
#include "proto/lan_share_protocol.h"
#include "utils/logger.h"
#include "net_trigger.h"

class SyncCliLogic : public LogicTcp, public LogicUdp
{
private:
    void handleHelloAck(LanSyncPkt &pkt);

public:
    enum state st;
    NetTrigger *discovery;


    SyncCliLogic(){};
    ~SyncCliLogic();
    void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void setDiscoveryTrigger(NetTrigger * tr);
    NetTrigger& getDiscoveryTrigger();
};

class SyncCliLogicTcp : public LogicTcp
{
private:
    LogicTcp &logic;

public:
    SyncCliLogicTcp(LogicTcp &logic) : logic(logic){};
    ~SyncCliLogicTcp();
    void recv_tcp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
};

class SyncCliLogicUdp : public LogicUdp
{
private:
    LogicUdp &logic;

public:
    SyncCliLogicUdp(LogicUdp &logic) : logic(logic){};
    ~SyncCliLogicUdp();
    void recv_udp(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
};

#endif