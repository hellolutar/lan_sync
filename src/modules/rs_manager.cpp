#include "rs_manager.h"

static RsLocalManager rlm("");
static RsSyncManager *rsm;
void ResourceManager::init(string home)
{
    if (home.size() == 0)
    {
        LOG_ERROR("ResourceManager::init() : strlen(home) is 0!");
        return;
    }

    LOG_INFO("ResourceManager::init() : resource.home is : {}", home);
    rlm = RsLocalManager(home);
    rsm = new RsSyncManager(rlm);
}
void ResourceManager::cleanup()
{
    rlm.setRsHomePath("");
    delete rsm;
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

RsSyncManager &ResourceManager::getRsSyncManager()
{
    if (rsm == nullptr)
    {
        LOG_ERROR("PLEASE run ResourceManager::init firstly!");
        exit(-1);
    }
    return *rsm;
}
