#ifndef __SYNC_CLI_SYNC_LOGIC_H
#define __SYNC_CLI_SYNC_LOGIC_H

#include "modules/net_trigger.h"
#include "proto/lan_share_protocol.h"

class SyncCliReqTbIdxLogic : public AbstNetConnTriggerBehavior
{
public:
    SyncCliReqTbIdxLogic(/* args */){};
    ~SyncCliReqTbIdxLogic(){};
    virtual void exec(NetworkConnCtx &ctx) override;
};

#endif