#ifndef __SYNC_CLI_DISCOVER_LOGIC_H_
#define __SYNC_CLI_DISCOVER_LOGIC_H_

#include "modules/net_trigger.h"
#include "proto/lan_share_protocol.h"

class SyncCliDiscoverLogic : public AbstNetConnTriggerBehavior
{
public:
    SyncCliDiscoverLogic(/* args */);
    ~SyncCliDiscoverLogic(){};
    void exec(NetworkConnCtx &ctx) override;
};


#endif