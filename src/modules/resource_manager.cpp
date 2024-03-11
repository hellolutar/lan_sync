#include "resource_manager.h"

#include <cstdlib>

using namespace std;

static ResourceManager rsm("");

void ResourceManager::init(string home)
{
    if (home.size() == 0)
    {
        LOG_ERROR("ResourceManager::init() : strlen(home) is 0!");
        return;
    }

    LOG_INFO("ResourceManager::init() : resource.home is : {}", home);
    rsm = ResourceManager(home);
}
ResourceManager &ResourceManager::getRsm()
{
    if (rsm.getRsHome() == "")
    {
        LOG_ERROR("PLEASE run ResourceManager::init firstly!");
        exit(-1);
    }
    return rsm;
}

ResourceManager::ResourceManager(string path)
{
    rsHome = path;
    last_update_time = filesystem::file_time_type::clock::time_point(std::chrono::seconds(0));
};

ResourceManager::~ResourceManager()
{
    freeTable();
}

void ResourceManager::freeTable()
{
    if (table.size() == 0)
        return;

    for (auto iter = table.begin();;)
    {
        free(iter->second);
        iter = table.erase(iter);
        if (table.size() == 0)
        {
            break;
        }
    }
}

vector<struct Resource *> ResourceManager::getTable()
{
    refreshTable();

    vector<Resource *> rss;
    for (auto i = table.begin(); i != table.end(); i++)
    {
        rss.push_back((*i).second);
    }

    return rss;
}

const struct Resource *ResourceManager::queryByUri(string uri)
{
    return table[uri];
}

void ResourceManager::refreshTable()
{
    vector<struct Resource *> newer = genResources(rsHome);
    for (auto i = 0; i < newer.size(); i++)
    {
        auto new_rs = newer[i];
        auto old_rs = table[new_rs->uri];
        if (old_rs != nullptr)
        {
            free(old_rs);
        }
        table[new_rs->uri] = new_rs;

        last_update_time = filesystem::file_time_type::clock::now();
    }
}

void ResourceManager::setRsHomePath(string path)
{
    rsHome = path;
}

vector<struct Resource *> ResourceManager::genResources(string pathStr)
{
    filesystem::path p(pathStr);
    return recurPath(p);
}

vector<struct Resource *> ResourceManager::recurPath(filesystem::path p)
{

    if (filesystem::is_regular_file(p))
    {
        string pstr = p.string();
        string filename = p.filename();
        string path = filesystem::absolute(p).string();
        uint64_t size = filesystem::file_size(p);
        filesystem::file_time_type fileLastWriteTime = filesystem::last_write_time(p);

        int index = path.find(rsHome) + rsHome.size();
        string uri = path.substr(index);

        auto tmprs = queryByUri(uri);
        if (tmprs != nullptr && fileLastWriteTime < last_update_time)
            return {};

        string hashRet = OpensslUtil::mdEncodeWithSHA3_512(pstr);
        if (hashRet.size() == 0)
            return {};

        struct Resource *res = (struct Resource *)malloc(sizeof(struct Resource));
        strcpy(res->name, filename.data());
        strcpy(res->path, path.data());
        strcpy(res->hash, hashRet.data());
        res->size = size;

        strcpy(res->uri, uri.data());

        return {res};
    }

    if (filesystem::is_directory(p))
    {
        vector<struct Resource *> paths;
        for (auto &&i : filesystem::directory_iterator(p))
        {
            auto ret = recurPath(i);
            paths.insert(paths.begin(), ret.begin(), ret.end());
        }
        return paths;
    }
    return {};
}

string ResourceManager::getRsHome()
{
    return rsHome;
}

bool ResourceManager::validRes(string uri, string hash)
{
    refreshTable();

    const struct Resource *rs = queryByUri(uri);
    if (rs == nullptr)
        return false;

    if (hash.compare(rs->hash) == 0)
    {
        updateSyncEntryStatus(uri, SUCCESS);
        return true;
    }
    else
        updateSyncEntryStatus(uri, FAIL);
    return false;
}

std::vector<struct Resource> ResourceManager::queryNeedToSync(struct Resource *table, uint64_t table_entry_num)
{
    vector<struct Resource> peer_table;

    for (size_t i = 0; i < table_entry_num; i++)
        peer_table.push_back(table[i]);

    return queryNeedToSync(peer_table);
}
std::vector<struct Resource> ResourceManager::queryNeedToSync(std::vector<struct Resource> peer_table)
{
    map<string, Resource> filtered;

    for (size_t i = 0; i < peer_table.size(); i++)
    {
        if (strlen(peer_table[i].uri) == 0)
            continue;

        filtered[peer_table[i].name] = peer_table[i];
    }

    if (filtered.size() == 0)
        return {};

    vector<struct Resource *> local_table = getTable();
    for (size_t i = 0; i < local_table.size(); i++)
    {
        struct Resource *local_rs = local_table[i];
        struct Resource rs = filtered[local_rs->name];
        if (strlen(rs.name) == 0 || rs.size < local_rs->size)
        {
            // I have the resource or my resource should sync to peer.
            filtered.erase(local_rs->name);
            continue;
        }
        if (rs.size == local_rs->size)
        {
            if (compareChar(rs.hash, local_rs->hash, strlen(rs.hash)))
                filtered.erase(local_rs->name);
            else
                LOG_WARN("[SYNC CLI] [{}] size is euqal, but hash is not equal\npeer:{}\nmine:{}", local_rs->uri, rs.hash, local_rs->hash);
        }
    }

    vector<struct Resource> want_to_sync;
    for (auto iter = filtered.begin(); iter != filtered.end(); iter++)
        want_to_sync.push_back(iter->second);

    return want_to_sync;
}

void ResourceManager::analysisThenUpdateSyncTable(struct Resource *table, uint64_t table_entry_num)
{
    std::vector<struct Resource> needsyncs = queryNeedToSync(table, table_entry_num);
    for (size_t i = 0; i < needsyncs.size(); i++)
    {
        Resource need_sync_rs = needsyncs[i];
        WantToSyncVO syncing_rs = syncTable[need_sync_rs.uri];
        if (syncing_rs.getUri().size() == 0)
        {
            syncTable[need_sync_rs.uri] = WantToSyncVO(need_sync_rs.uri, PENDING, Range(0, need_sync_rs.size));
            continue;
        }

        // compare size
        if (syncing_rs.getRange().getSize() > 0)
        {
            // TODO:  this server the resouce is bigger, now need to get resource from this server, and discard the old server
            if (need_sync_rs.size > syncing_rs.getRange().getSize())
            {
                LOG_WARN("TODO: ResourceManager::analysisThenUpdateSyncTable() this server the resouce is bigger, now need to get resource from this server, and discard the old server");

                LOG_INFO("[SYNC CLI] SYNC RESET: URI:{}", syncing_rs.getUri().data());
                syncing_rs.setStatus(RESET);
            }
        }
    }
}

map<string, WantToSyncVO> &ResourceManager::getSyncTable()
{
    return syncTable;
}

string ResourceManager::resourcePosition(string uri)
{
    return getRsHome() + uri;
}

bool ResourceManager::saveLocal(string uri, void *data, uint64_t offset, uint64_t data_len)
{
    LOG_INFO("[SYNC CLI] SYNC save to local: offset:{} data_len:{} URI:{}", offset, data_len, uri);
    string pathstr = resourcePosition(uri);

    auto path = filesystem::path(pathstr);
    auto ppath = filesystem::absolute(path).parent_path();
    if (!filesystem::exists(ppath))
    {
        filesystem::create_directories(path.parent_path());
    }

    uint64_t ret = io.writeFile(pathstr, offset, data, data_len);
    if (ret < 0)
        return false;

    return true;
}

void ResourceManager::updateSyncEntryStatus(std::string uri, WantSyncResourceStatusEnum status)
{
    if (syncTable[uri].getUri().size() > 0)
        syncTable[uri].setStatus(status);

    LOG_INFO("ResourceManager::updateSyncEntryStatus [{}]: URI:{}", WantSyncResourceStatusEnumToString(status), uri);
    switch (status)
    {
    case SUCCESS:
        delSyncEntry(uri);
        break;
    case FAIL:
        std::filesystem::path p(resourcePosition(uri).data());
        if (filesystem::is_regular_file(p))
            filesystem::remove(p);
        break;
    }
}

void ResourceManager::updateSyncEntryLastUpteTime(std::string uri, time_t t)
{
    if (syncTable[uri].getUri().size() > 0)
        syncTable[uri].setLastUpdateTime(t);
}

void ResourceManager::delSyncEntry(std::string uri)
{
    syncTable.erase(uri);
}