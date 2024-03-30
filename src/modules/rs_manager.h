#ifndef __RS_MANAGER_H_
#define __RS_MANAGER_H_

#include <string>

#include "rs_local_manager.h"
#include "rs_sync_mamanger.h"

class ResourceManager
{
public:
    static RsLocalManager &getRsLocalManager();
    static RsSyncManager &getRsSyncManager();
    static void init(std::string home);
    static void cleanup();

    static void save(NetAddr &peer,std::string uri, void *data, uint64_t offset, uint64_t data_len);
};




#endif