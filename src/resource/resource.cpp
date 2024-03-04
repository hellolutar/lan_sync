#include "resource.h"

struct WantSyncResource *WantSyncResource_new(struct bufferevent *bev, string uri, enum WantSyncResourceStatusEnum status, uint64_t end_pos)
{
    struct WantSyncResource *syncRs = (struct WantSyncResource *)malloc(sizeof(struct WantSyncResource));  
    syncRs->bev = bev;
    memcpy(syncRs->uri, uri.data(), NAME_MAX_SIZE);
    syncRs->status = status;
    syncRs->last_update_time = time(0);
    Range range(0, 0);
    syncRs->range = range;

    return syncRs;
};

struct Resource *resource_convert_vec_to_arr(vector<struct Resource *> table)
{
    uint32_t num = table.size();
    uint32_t size = num * sizeof(struct Resource);
    struct Resource *ret = (struct Resource *)malloc(size);
    struct Resource *retp = ret;
    memset(ret, 0, size);
    for (int i = 0; i < table.size(); i++)
    {
        memcpy(&(retp[i].name), table[i]->name, strlen(table[i]->name));
        memcpy(&(retp[i].uri), table[i]->uri, strlen(table[i]->uri));
        memcpy(&(retp[i].path), table[i]->path, strlen(table[i]->path));
        memcpy(&(retp[i].hash), table[i]->hash, strlen(table[i]->hash));
        retp[i].size = table[i]->size;
    }

    return ret;
}

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