#include "rs_local_manager.h"

#include <cstdlib>

using namespace std;

RsLocalManager::RsLocalManager(string path)
{
    rsHome = path;
};

RsLocalManager::~RsLocalManager()
{
    freeTable();
}

void RsLocalManager::freeTable()
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

vector<struct Resource *> RsLocalManager::getTable()
{
    refreshTable();

    vector<Resource *> rss;
    for (auto i = table.begin(); i != table.end(); i++)
    {
        rss.push_back((*i).second);
    }

    return rss;
}

const struct Resource *RsLocalManager::queryByUri(string uri)
{
    return table[uri];
}

void RsLocalManager::refreshTable()
{
    table.clear();
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
    }
}

void RsLocalManager::setRsHomePath(string path)
{
    rsHome = path;
}

vector<struct Resource *> RsLocalManager::genResources(string pathStr)
{
    filesystem::path p(pathStr);
    return recurPath(p);
}

vector<struct Resource *> RsLocalManager::recurPath(filesystem::path p)
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
        if (tmprs != nullptr)
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

string RsLocalManager::getRsHome()
{
    return rsHome;
}

bool RsLocalManager::validRes(string uri, string hash)
{
    refreshTable();

    const struct Resource *rs = queryByUri(uri);
    if (rs == nullptr)
        return false;

    if (hash.compare(rs->hash) == 0)
    {
        return true;
    }
    else
        return false;
}

std::vector<struct Resource> RsLocalManager::cmpThenRetNeedToSyncTable(struct Resource *table, uint64_t table_entry_num)
{
    vector<struct Resource> peer_table;

    for (size_t i = 0; i < table_entry_num; i++)
        peer_table.push_back(table[i]);

    return cmpThenRetNeedToSyncTable(peer_table);
}
std::vector<struct Resource> RsLocalManager::cmpThenRetNeedToSyncTable(std::vector<struct Resource> peer_table)
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
        else if (rs.size == local_rs->size)
        {
            if (compareChar(rs.hash, local_rs->hash, strlen(rs.hash)))
                filtered.erase(local_rs->name);
            else
                LOG_DEBUG("RsLocalManager::cmpThenRetNeedToSyncTable() : uri[{}] need to add to synctable, reason:{}", rs.uri, "hash is not eq!");
        }
        else
            LOG_DEBUG("RsLocalManager::cmpThenRetNeedToSyncTable() : uri[{}] need to add to synctable, reason:{}", rs.uri, "size: peer[{}] > local[{}]", rs.size, local_rs->size);
    }

    vector<struct Resource> want_to_sync;
    for (auto iter = filtered.begin(); iter != filtered.end(); iter++)
        want_to_sync.push_back(iter->second);

    return want_to_sync;
}

string RsLocalManager::resourcePosition(string uri)
{
    return getRsHome() + uri;
}

bool RsLocalManager::saveLocal(string uri, void *data, uint64_t offset, uint64_t data_len)
{
    LOG_DEBUG("RsLocalManager::saveLocal() : URI:{} offset:{} data_len:{}", uri, offset, data_len);
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
