#include "resource_manager.h"

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
    if (table.size() ==0)
        return;

    for (auto it : table)
    {
        table.erase(it.first);
        free(it.second);
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
    return recurPath(filesystem::absolute(p));
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

        string hashRet = opensslUtil.mdEncodeWithSHA3_512(pstr);
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

bool ResourceManager::checkHash(string uri, string hash)
{
    refreshTable();

    const struct Resource *rs = queryByUri(uri);
    if (rs == nullptr)
        return false;

    return hash.compare(rs->hash) == 0;
}