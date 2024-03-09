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

void WantToSyncVO::setStatus(WantSyncResourceStatusEnum status)
{
    this->status = status;
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