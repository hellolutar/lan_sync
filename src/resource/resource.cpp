#include "resource.h"

struct WantSyncResource *WantSyncResource_new(struct bufferevent *bev, string uri, enum WantSyncResourceStatusEnum status)
{
    struct WantSyncResource *syncRs = new WantSyncResource();
    syncRs->bev = bev;
    syncRs->uri = uri;
    syncRs->status = status;
    syncRs->last_update_time = time(0);

    return syncRs;
};