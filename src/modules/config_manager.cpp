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
void ConfigManager::reload(std::string path)
{
    p = PropertiesParse(path);
    config_at = path;
}
