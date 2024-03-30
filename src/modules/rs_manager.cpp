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

void ResourceManager::save(NetAddr &peer, std::string uri, void *data, uint64_t offset, uint64_t data_len)
{
    bool write_ret = rlm.saveLocal(uri, data, offset, data_len);

    Block b(offset, offset + data_len);
    if (!write_ret)
    {
        LOG_INFO("SyncService::handleLanSyncReplyResource() : {} : block save fail:[{},{})", uri, b.start, b.end);
        rsm->unregReqSyncRsByBlock(peer, b, uri);
    }
    else
    {
        LOG_INFO("SyncService::handleLanSyncReplyResource() : {} : block save success:[{},{})", uri, b.start, b.end);
        rsm->syncingRangeDoneAndValid(peer, uri, b, true);
    }
}