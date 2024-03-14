#include "rs_sync_mamanger.h"

using namespace std;

bool operator==(const Block &l, const Block &r)
{
    return l.start == r.start && l.end == r.end;
}

bool Block::operator==(const Block &other)
{
    return start == other.start && end == other.end;
}

SyncRs::SyncRs()
{
}

SyncRs::SyncRs(std::string uri, std::uint64_t size, std::string hash, NetAddr owner)
{
    this->uri = uri;
    this->size = size;
    this->hash = hash;
    this->owner.push_back(owner);

    if (size <= SIZE_1MByte)
    {
        this->block.push_back(Block(0, size + 1));
    }
    else
    {
        uint num = size / SIZE_1MByte + 1; // 向上取整
        uint64_t offset = 0;
        for (uint i = 0; i < num; i++)
        {
            uint64_t end = min(size + 1, offset + SIZE_1MByte + 1);
            block.push_back(Block(offset, end));
            offset = end;
            if (offset > size)
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

RsSyncManager::RsSyncManager(/* args */)
{
}

RsSyncManager::~RsSyncManager()
{
}

std::map<std::string, SyncRs> &RsSyncManager::getAllUriRs()
{
    return uriRs;
}

void RsSyncManager::refreshSyncingRsByTbIdx(NetAddr peer, struct Resource *table, uint64_t rs_size)
{
    RsLocalManager &rm = ResourceManager::getRsLocalManager();

    std::vector<struct Resource> need_to_sync_table = rm.cmpThenRetNeedToSyncTable(table, rs_size);
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
                uriRs[uri].owner.push_back(peer);
                return true;
            }
        }
    }
    return true;
}

Block RsSyncManager::regReqSyncRsAuto(NetAddr peer, string uri)
{

    if (uriRs[uri].block.size() > 0)
    {
        uint blocksize = uriRs[uri].block.size();
        Block block = uriRs[uri].block[blocksize - 1];

        uriRs[uri].block.pop_back();

        SyncingRange req(peer, block);
        uriRs[uri].syncing.push_back(req);
        return {block};
    }
    return {};
}

void RsSyncManager::unregReqSyncRs(NetAddr peer, std::string uri)
{
    for (auto iter = uriRs[uri].syncing.end() - 1; iter >= uriRs[uri].syncing.begin(); iter--)
    {
        SyncingRange rng = *iter;
        if (rng.peer == peer)
        {
            uriRs[uri].block.push_back(rng.block);
            uriRs[uri].syncing.erase(iter);
        }
    }

    for (auto iter = uriRs[uri].owner.end() - 1; iter >= uriRs[uri].owner.begin(); iter--)
    {
        if ((*iter) == peer)
        {
            uriRs[uri].owner.erase(iter);
        }
    }
}

void RsSyncManager::syncingRangeDone(NetAddr peer, string uri, Block block)
{
    for (auto iter = uriRs[uri].syncing.end(); iter < uriRs[uri].syncing.begin(); iter--)
    {
        SyncingRange syncing = (*iter);
        if (syncing.peer == peer)
        {
            if (syncing.block == block)
            {
                uriRs[uri].syncing.erase(iter);
                break;
            }
        }
    }
}
