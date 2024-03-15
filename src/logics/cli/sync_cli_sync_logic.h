#ifndef __SYNC_CLI_SYNC_LOGIC_H
#define __SYNC_CLI_SYNC_LOGIC_H

#include "modules/net_trigger.h"
#include "modules/rs_manager.h"
#include "utils/logger.h"
#include "proto/lan_share_protocol.h"
#include "components/buf_base_on_event.h"

class SyncCliSyncLogic : public AbstNetConnTriggerBehavior
{
public:
    SyncCliSyncLogic(/* args */){};
    ~SyncCliSyncLogic(){};
    virtual void exec(NetworkConnCtx &ctx) override;
    void reqTbIdx(NetworkConnCtx &ctx);
    void reqRs(NetworkConnCtx &ctx);
    void sendReqRs(NetworkConnCtx &ctx, string uri, Block b);
};

#endif