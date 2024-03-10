#ifndef __CONFIG_MANAGER_H_
#define __CONFIG_MANAGER_H_

#include <string>

#include "utils/logger.h"
#include "utils/properties_parse.h"

class ConfigManager
{
private:
    ConfigManager(/* args */){};
    ~ConfigManager(){};
    static std::string config_at;
    static PropertiesParse p;

public:
    static std::string query(std::string key);
    static void reload(std::string path);
};

#endif