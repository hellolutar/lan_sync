#include "rs_sync_mamanger.h"

using namespace std;

SyncRs::SyncRs()
{
}

SyncRs::SyncRs(std::string uri, std::uint64_t size, std::string hash, NetAddr owner)
{
    this->uri = uri;
    this->size = size;
    this->hash = hash;
    this->owner.push_back(owner);

    if (size <= BLOCK_SIZE)
    {
        this->block.push_back(Block(0, size));
    }
    else
    {
        uint num = size / BLOCK_SIZE + 1; // 向上取整
        uint64_t offset = 0;
        for (uint i = 0; i < num; i++)
        {
            uint64_t end = min(size, offset + BLOCK_SIZE);
            block.push_back(Block(offset, end));
            offset = end;
            if (offset >= size)
            {
                break;
            }
        }
    }
}

SyncingRange::SyncingRange(NetAddr peer, Block block)
{
    this->peer = peer;
    this->block = block;

    uint64_t size = block.end - block.start;

    time_t now;
    time(&now);
    this->timeOut = now + size / SIZE_50_KByte;
}

std::map<std::string, SyncRs> &RsSyncManager::getAllUriRs()
{
    // TODO(lutar, 20240317) there is a bug for multi thread case
    return uriRs;
}

void RsSyncManager::refreshSyncingRsByTbIdx(NetAddr peer, struct Resource *table, uint64_t rs_size)
{
    std::vector<struct Resource> need_to_sync_table = rlm.cmpThenRetNeedToSyncTable(table, rs_size);
    for (auto i = 0; i < need_to_sync_table.size(); i++)
    {
        Resource rs = need_to_sync_table[i];
        regSyncRs(peer, rs.uri, rs.hash, rs.size);
    }
}

bool RsSyncManager::regSyncRs(NetAddr peer, std::string uri, std::string hash, uint64_t size)
{
    // 1.查询uri， 比较size， 更新
    if (uriRs.find(uri) == uriRs.end())
    {
        SyncRs rs(uri, size, hash, peer);
        uriRs[uri] = rs;
        return true;
    }
    else
        return updateSyncRs(peer, uri, hash, size);
}

bool RsSyncManager::updateSyncRs(NetAddr peer, std::string uri, std::string hash, uint64_t size)
{
    scoped_lock<std::mutex> lk(mut);
    if (uriRs.find(uri) == uriRs.end())
    {
        SyncRs rs(uri, size, hash, peer);
        uriRs[uri] = rs;
        return true;
    }
    else
    {
        if (uriRs[uri].size < size)
        {
            uriRs[uri] = SyncRs(uri, size, hash, peer);
            return true;
        }
        else if (uriRs[uri].size > size)
        {
            return false;
        }
        else
        {
            if (uriRs[uri].hash != hash)
                return false;
            else
            {
                for (auto o : uriRs[uri].owner)
                {
                    if (o == peer)
                        return true;
                }
                uriRs[uri].owner.push_back(peer);
                return true;
            }
        }
    }
    return true;
}

Block RsSyncManager::regReqSyncRsAuto(NetAddr peer, string uri)
{
    scoped_lock<std::mutex> lk(mut);
    if (uriRs[uri].block.size() > 0)
    {
        for (uint64_t i = 0; i < uriRs[uri].owner.size(); i++)
        {
            if (uriRs[uri].owner[i] == peer)
            {
                uint blocksize = uriRs[uri].block.size();
                Block block = uriRs[uri].block[blocksize - 1];

                uriRs[uri].block.pop_back();

                SyncingRange req(peer, block);
                uriRs[uri].syncing.push_back(req);
                return block;
            }
        }
    }
    return Block(0, 0);
}

void RsSyncManager::unregReqSyncRsByBlock(NetAddr peer, Block b, std::string uri)
{
    scoped_lock<std::mutex> lk(mut);
    for (auto iter = uriRs[uri].syncing.end() - 1; iter >= uriRs[uri].syncing.begin(); iter--)
    {
        SyncingRange rng = *iter;
        if (rng.peer == peer && rng.block == b)
        {
            uriRs[uri].block.push_back(rng.block);
            uriRs[uri].syncing.erase(iter);
            break;
        }
    }
}

void RsSyncManager::unregAllReqSyncRsByPeer(NetAddr peer, std::string uri)
{
    scoped_lock<std::mutex> lk(mut);
    for (auto iter = uriRs[uri].syncing.end() - 1; iter >= uriRs[uri].syncing.begin(); iter--)
    {
        SyncingRange rng = *iter;
        if (rng.peer == peer)
        {
            LOG_INFO("RsSyncManager::unregAllReqSyncRsByPeer() : [{}] CONNECTION ERROR, Remove related resources!");
            uriRs[uri].block.push_back(rng.block);
            uriRs[uri].syncing.erase(iter);
        }
    }

    for (auto iter = uriRs[uri].owner.end() - 1; iter >= uriRs[uri].owner.begin(); iter--)
    {
        if ((*iter) == peer)
        {
            LOG_INFO("RsSyncManager::unregAllReqSyncRsByPeer() : [{}] CONNECTION ERROR, Disconnect ip from related resources!");
            uriRs[uri].owner.erase(iter);
        }
    }
}

void RsSyncManager::syncingRangeDoneAndValid(NetAddr peer, string uri, Block block, bool valid)
{
    scoped_lock<std::mutex> lk(mut);
    for (auto iter = uriRs[uri].syncing.end() - 1; iter >= uriRs[uri].syncing.begin(); iter--)
    {
        SyncingRange syncing = (*iter);
        if (syncing.peer == peer)
        {
            if (syncing.block == block)
            {
                uriRs[uri].syncing.erase(iter);

                // sync complete
                if (uriRs[uri].block.size() == 0 && uriRs[uri].syncing.size() == 0)
                {
                    LOG_INFO("RsSyncManager::syncingRangeDoneAndValid() : [{}] SYNC COMPLETE!", uri);
                    uriRs[uri].success = true;
                    if (valid)
                    {
                        bool ret = rlm.validRes(uri, uriRs[uri].hash);
                        if (ret)
                            LOG_INFO("RsSyncManager::syncingRangeDoneAndValid() : [{}] SYNC SUCCESS! HASH IS VALID!", uri);
                        else
                            LOG_WARN("RsSyncManager::syncingRangeDoneAndValid() : [{}] SYNC FAIL! HASH IS INVALID!", uri);
                    }
                    uriRs.erase(uri);
                }
                break;
            }
            else
            {
                LOG_INFO("RsSyncManager::syncingRangeDoneAndValid() : block is not eq!");
                // 需要一个buffer。
            }
        }
    }
}
void RsSyncManager::syncingRangeDone(NetAddr peer, string uri, Block block)
{
    syncingRangeDoneAndValid(peer, uri, block, false);
}
