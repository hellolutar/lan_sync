#ifndef __REOURCE_H_
#define __REOURCE_H_

#include <cstdint>

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

#endif
