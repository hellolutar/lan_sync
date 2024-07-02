#ifndef __RS_MANAGER_H_
#define __RS_MANAGER_H_

#include <string>

#include "modules/rs_local_manager.h"

class ResourceManager
{
public:
    static RsLocalManager &getRsLocalManager();
    static void init(std::string home);
    static void cleanup();
};

#endif