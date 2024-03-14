#include "rs_manager.h"

static RsLocalManager rsm("");

void ResourceManager::init(string home)
{
    if (home.size() == 0)
    {
        LOG_ERROR("ResourceManager::init() : strlen(home) is 0!");
        return;
    }

    LOG_INFO("ResourceManager::init() : resource.home is : {}", home);
    rsm = RsLocalManager(home);
}
RsLocalManager &ResourceManager::getRsLocalManager()
{
    if (rsm.getRsHome() == "")
    {
        LOG_ERROR("PLEASE run ResourceManager::init firstly!");
        exit(-1);
    }
    return rsm;
}
