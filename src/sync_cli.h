#ifndef __SYNC_CLI_H_
#define __SYNC_CLI_H_

#include "modules/discovery_trigger.h"

class SyncCli
{
private:

public:
    DiscoveryTrigger *discovery;
    
    SyncCli();
    ~SyncCli();
    void setDiscoveryTrigger(DiscoveryTrigger *tr);
};

#endif