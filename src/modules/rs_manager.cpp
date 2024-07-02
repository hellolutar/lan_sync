#include "rs_manager.h"

static RsLocalManager rlm("");
void ResourceManager::init(string home)
{
    if (home.size() == 0)
    {
        LOG_ERROR("ResourceManager::init() : strlen(home) is 0!");
        exit(-1);
    }

    LOG_INFO("ResourceManager::init() : resource.home is : {}", home);
    rlm = RsLocalManager(home);
}
void ResourceManager::cleanup()
{
    rlm.setRsHomePath("");
}
RsLocalManager &ResourceManager::getRsLocalManager()
{
    if (rlm.getRsHome() == "")
    {
        LOG_ERROR("PLEASE run ResourceManager::init firstly!");
        exit(-1);
    }
    return rlm;
}
