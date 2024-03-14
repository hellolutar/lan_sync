#ifndef __SYNC_CLI_SYNC_LOGIC_H
#define __SYNC_CLI_SYNC_LOGIC_H

#include "modules/net_trigger.h"
#include "modules/rs_local_manager.h"
#include "utils/logger.h"
#include "proto/lan_share_protocol.h"

class SyncCliSyncLogic : public AbstNetConnTriggerBehavior
{
public:
    SyncCliSyncLogic(/* args */){};
    ~SyncCliSyncLogic(){};
    virtual void exec(NetworkConnCtx &ctx) override;
    void hdl_pending(NetworkConnCtx &ctx, SyncRsVO vo);
    void hdl_syncing(SyncRsVO vo);
    void reqTbIdx(NetworkConnCtx &ctx);
    void reqRs(NetworkConnCtx &ctx);
};

#endif