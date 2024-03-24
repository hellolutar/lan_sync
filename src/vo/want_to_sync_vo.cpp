#include "want_to_sync_vo.h"

using namespace std;

string WantSyncResourceStatusEnumToString(WantSyncResourceStatusEnum status)
{
    switch (status)
    {
    case PENDING:
        return "PENDING";
    case SYNCING:
        return "SYNCING";
    case SUCCESS:
        return "SUCCESS";
    case FAIL:
        return "FAIL";
    default:
        return "UNKOWN";
    }
}

SyncRsVO::SyncRsVO(std::string uri, enum WantSyncResourceStatusEnum status, Range range)
{
    this->uri = uri;
    this->status = status;
    this->range = range;
    int delay = range.getSize() / SIZE_50_KB;
    this->max_delay = max(delay, 60);
    time(&(this->last_update_time));
}

void SyncRsVO::setStatus(WantSyncResourceStatusEnum status)
{
    this->status = status;
}

void SyncRsVO::setLastUpdateTime(time_t t)
{
    this->last_update_time = t;
}

string SyncRsVO::getUri()
{
    return uri;
}
enum WantSyncResourceStatusEnum SyncRsVO::getStatus()
{
    return status;
}
Range SyncRsVO::getRange()
{
    return range;
}
time_t SyncRsVO::getLast_update_time()
{
    return last_update_time;
}

uint32_t SyncRsVO::getMaxDelay()
{
    return max_delay;
}


SyncPeer::~SyncPeer()
{
    delete syncvo;
}