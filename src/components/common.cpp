#include "common.h"

void load_config(int argc, char const *argv[])
{
    std::stringstream ss;
    ss << std::filesystem::current_path().string() << "/"
       << CONFIG_DEFAULT_NAME;
    std::string default_config_file;
    ss >> default_config_file;

    cxxopts::Options options("sync", "A sync program that sync file.");

    options.add_options()("c,config", "config path", cxxopts::value<std::string>()->default_value(default_config_file));

    auto result = options.parse(argc, argv);
    if (argc == 1 || result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    std::string config_at = result["config"].as<std::string>();
    ConfigManager::reload(config_at);

    ResourceManager::init(ConfigManager::query(CONFIG_KEY_RESOURCE_HOME));
}