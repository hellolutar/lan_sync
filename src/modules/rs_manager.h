#ifndef __RS_MANAGER_H_
#define __RS_MANAGER_H_

#include <string>

#include "rs_local_manager.h"

class ResourceManager
{
private:
    /* data */
public:
    static RsLocalManager &getRsLocalManager();
    static void init(std::string home);
};




#endif