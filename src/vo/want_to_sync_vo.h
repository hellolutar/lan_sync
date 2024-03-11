#ifndef __WANT_TO_SYNC_VO_H_
#define __WANT_TO_SYNC_VO_H_

#include <string>
#include "proto/lan_share_protocol.h"
#include "vo/dto/resource.h"

#define SIZE_50_KB 51200

enum WantSyncResourceStatusEnum
{
    PENDING = 1,
    SYNCING,
    SUCCESS,
    FAIL,
    RESET,
};

std::string WantSyncResourceStatusEnumToString(WantSyncResourceStatusEnum status);

class WantToSyncVO
{
private:
    std::string uri = "";
    enum WantSyncResourceStatusEnum status = FAIL;
    Range range = Range(0, 0);
    time_t last_update_time = 0;
    uint32_t max_delay = 250;

public:
    WantToSyncVO() {}
    WantToSyncVO(std::string uri, enum WantSyncResourceStatusEnum status, Range range);
    ~WantToSyncVO(){};

    void setStatus(WantSyncResourceStatusEnum status);
    void setLastUpdateTime(time_t t);
    std::string getUri();
    enum WantSyncResourceStatusEnum getStatus();
    Range getRange();
    time_t getLast_update_time();
    uint32_t getMaxDelay(); 
};

#endif