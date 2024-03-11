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

WantToSyncVO::WantToSyncVO(std::string uri, enum WantSyncResourceStatusEnum status, Range range)
{
    this->uri = uri;
    this->status = status;
    this->range = range;
    int delay = range.getSize() / SIZE_50_KB;
    this->max_delay = max(delay, 60);
    time(&(this->last_update_time));
}

void WantToSyncVO::setStatus(WantSyncResourceStatusEnum status)
{
    this->status = status;
}

void WantToSyncVO::setLastUpdateTime(time_t t)
{
    this->last_update_time = t;
}

string WantToSyncVO::getUri()
{
    return uri;
}
enum WantSyncResourceStatusEnum WantToSyncVO::getStatus()
{
    return status;
}
Range WantToSyncVO::getRange()
{
    return range;
}
time_t WantToSyncVO::getLast_update_time()
{
    return last_update_time;
}

uint32_t WantToSyncVO::getMaxDelay()
{
    return max_delay;
}