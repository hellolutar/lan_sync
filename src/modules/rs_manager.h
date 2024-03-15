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
};




#endif