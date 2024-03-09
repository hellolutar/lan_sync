#ifndef __WANT_TO_SYNC_VO_H_
#define __WANT_TO_SYNC_VO_H_

#include <string>
#include "proto/lan_share_protocol.h"
#include "vo/dto/resource.h"

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

public:
    WantToSyncVO(){}
    WantToSyncVO(std::string uri, enum WantSyncResourceStatusEnum status, Range range)
        : uri(uri), status(status), range(range), last_update_time(time(0)) {}
    ~WantToSyncVO(){};

    void setStatus(WantSyncResourceStatusEnum status);
    std::string getUri();
    enum WantSyncResourceStatusEnum getStatus();
    Range getRange();
    time_t getLast_update_time();
};

#endif