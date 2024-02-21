#include "resource.h"

struct WantSyncResource *WantSyncResource_new(struct evbuffer *out, string uri, enum WantSyncResourceStatusEnum status)
{
    struct WantSyncResource *syncRs = new WantSyncResource();
    syncRs->out = out;
    syncRs->uri = uri;
    syncRs->status = status;
    syncRs->last_update_time = time(0);

    return syncRs;
};