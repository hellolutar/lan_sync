#include "resource_manager.h"

ResourceManager::~ResourceManager()
{
    freeTable();
}

void ResourceManager::freeTable()
{
    if (table.size() > 0)
    {
        for (int i = 0; i < table.size(); i++)
        {
            free(table.at(i));
        }
    }
}

vector<struct Resource *> ResourceManager::getTable()
{
    if (table.size() == 0)
        table = genResources(rsHome);

    return table;
}

void ResourceManager::refreshTable()
{
    freeTable();
    table = genResources(rsHome);
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

        string hashRet = opensslUtil.mdEncodeWithSHA3_512(pstr);
        if (hashRet.size() == 0)
        {
            return {};
        }

        struct Resource *res = (struct Resource *)malloc(sizeof(struct Resource));
        strcpy(res->name, filename.data());
        strcpy(res->path, path.data());
        strcpy(res->hash, hashRet.data());
        res->size = size;

        int index = path.find(rsHome) + rsHome.size();
        strcpy(res->uri, path.substr(index).data());

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
