#ifndef __RESOURCE_MANAGER_H
#define __RESOURCE_MANAGER_H

#include <cstdint>

#define RESOURCE_HOME / home / lutar / code / libevent / mine / sdn_sync / resources

/**
 * | name | size | digest | uri | path     |
 */
struct Resource
{
    char name[256];
    uint64_t size;
    // digest
    char uri[2048];
    char path[2048];
};

class ResourceManager
{
private:
public:
    ResourceManager(/* args */);
    ~ResourceManager();
};

#endif