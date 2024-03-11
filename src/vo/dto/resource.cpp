#include "resource.h"
#include <cstdlib>
#include <cstring>

struct Resource *Resource::vecToArr(std::vector<struct Resource *> rs_table)
{
    uint32_t size = rs_table.size() * sizeof(struct Resource);
    struct Resource *ret = (struct Resource *)malloc(size);
    struct Resource *retp = ret;
    memset(ret, 0, size);
    for (int i = 0; i < rs_table.size(); i++)
    {
        memcpy(&(retp[i].name), rs_table[i]->name, strlen(rs_table[i]->name));
        memcpy(&(retp[i].uri), rs_table[i]->uri, strlen(rs_table[i]->uri));
        memcpy(&(retp[i].path), rs_table[i]->path, strlen(rs_table[i]->path));
        memcpy(&(retp[i].hash), rs_table[i]->hash, strlen(rs_table[i]->hash));
        retp[i].size = rs_table[i]->size;
    }
    return ret;
}
