#ifndef __RESOURCE_MANAGER_H
#define __RESOURCE_MANAGER_H

#include <cstdint>
#include <cstring>
#include <malloc.h>
#include <vector>
#include <string>
#include <filesystem>

#include "../utils/openssl_utils.h"
#include "resource.h"

using namespace std;



class ResourceManager
{
private:
    vector<struct Resource *> table;
    vector<struct Resource *> recurPath(filesystem::path p);
    vector<struct Resource *> genResources(string p);
    void freeTable();
    string rsHome = nullptr;

    OpensslUtil opensslUtil;

public:
    ResourceManager(string path):rsHome(path){};
    ~ResourceManager();

    vector<struct Resource *> getTable();

    const struct Resource* queryByUri(string uri);
    string getRsHome();




    void refreshTable();

    void setRsHomePath(string path);
};

#endif