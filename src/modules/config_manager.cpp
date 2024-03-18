#include "config_manager.h"

using namespace std;

std::string ConfigManager::config_at = "";
PropertiesParse ConfigManager::p = PropertiesParse("");

std::string ConfigManager::query(std::string key)
{
    if (config_at.size() == 0)
    {
        LOG_ERROR("please run ConfigManager::reload(std::string path) firstly!");
        exit(-1);
    }

    return p.query(key);
}

std::vector<std::string> ConfigManager::queryList(std::string key)
{
    string value = query(key);
    vector<string> l;
    if (value.size() == 0)
    {
        return l;
    }

    if (value.find(",") == string::npos)
    {
        l.push_back(value);
        return l;
    }

    while (value.find(",") != string::npos)
    {
        size_t eq_pos = value.find(",");
        string ip = value.substr(0, eq_pos);
        l.push_back(ip);

        value = value.substr(eq_pos + 1, value.size() - (eq_pos + 1));
        if (value.size() > 0 && value.find(",")==string::npos)
        {
            l.push_back(value);
        }
    }
    return l;
}

void ConfigManager::reload(std::string path)
{
    p = PropertiesParse(path);
    config_at = path;
}
