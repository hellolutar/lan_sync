#ifndef __REOURCE_H_
#define __REOURCE_H_

#include <cstdint>
/**
 * | name | size | digest | uri | path     |
 */
struct Resource
{
    char name[2048];
    char uri[2048];
    char path[2048];
    char hash[2048];
    uint64_t size;
};

#endif
