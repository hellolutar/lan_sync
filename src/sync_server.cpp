#include "sync_server.h"

void configure(int argc, char const *argv[])
{
    stringstream ss;
    ss << filesystem::current_path().string() << "/"
       << CONFIG_DEFAULT_NAME;
    string default_config_file;
    ss >> default_config_file;

    cxxopts::Options options("sync", "A sync program that sync file.");
    options.add_options()("h,help", "Print usage");
    options.add_options()("c,config", "config path", cxxopts::value<string>()->default_value(default_config_file));

    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    string config_at = result["config"].as<std::string>();
    ConfigManager::reload(config_at);

    ResourceManager::init(ConfigManager::query(CONFIG_KEY_RESOURCE_HOME));
}

int main(int argc, char const *argv[])
{
    configure(argc, argv);

    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(base);

    SyncSrvLogic main_logic;
    LogicTcp &tcplogic = main_logic;
    LogicUdp &udplogic = main_logic;

    NetFrameworkImplWithEvent::addUdpServer(new NetSrvLogicContainer(NetAddr(":58080"), udplogic));
    NetFrameworkImplWithEvent::addTcpServer(new NetSrvLogicContainer(NetAddr(":58081"), tcplogic));

    NetFrameworkImplWithEvent::run();

    NetFrameworkImplWithEvent::free();

    return 0;
}