#ifndef __SYNC_CLI_SYNC_LOGIC_H
#define __SYNC_CLI_SYNC_LOGIC_H

#include "modules/net_trigger.h"
#include "modules/resource_manager.h"
#include "utils/logger.h"
#include "proto/lan_share_protocol.h"

class SyncCliReqTbIdxLogic : public AbstNetConnTriggerBehavior
{
public:
    SyncCliReqTbIdxLogic(/* args */){};
    ~SyncCliReqTbIdxLogic(){};
    virtual void exec(NetworkConnCtx &ctx) override;
};

class SyncCliReqRsLogic : public AbstNetConnTriggerBehavior
{
public:
    SyncCliReqRsLogic(/* args */){};
    ~SyncCliReqRsLogic(){};
    virtual void exec(NetworkConnCtx &ctx) override;
    void hdl_pending(NetworkConnCtx &ctx, WantToSyncVO vo);
    void hdl_syncing(WantToSyncVO vo);
};

#endif