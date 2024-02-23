#ifndef __REOURCE_H_
#define __REOURCE_H_

#include <cstdint>
#include <string>

#include <malloc.h>

using namespace std;

#define NAME_MAX_SIZE 2048

/**
 * | name | size | digest | uri | path     |
 */
struct Resource
{
    char name[NAME_MAX_SIZE];
    char uri[NAME_MAX_SIZE];
    char path[NAME_MAX_SIZE];
    char hash[NAME_MAX_SIZE];
    uint64_t size;
};

enum WantSyncResourceStatusEnum
{
    PENDING = 1,
    SYNCING,
    SUCCESS,
    FAIL,
};

struct WantSyncResource
{
    struct bufferevent *bev;
    string uri;
    enum WantSyncResourceStatusEnum status;
    time_t last_update_time;
};

struct WantSyncResource *WantSyncResource_new(struct bufferevent *bev, string uri, enum WantSyncResourceStatusEnum status);

#endif
