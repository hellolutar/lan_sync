#include "sync_cli.h"

void configure(int argc, char const *argv[])
{
    stringstream ss;
    ss << filesystem::current_path().string() << "/"
       << CONFIG_DEFAULT_NAME;
    string default_config_file;
    ss >> default_config_file;

    cxxopts::Options options("sync", "A sync program that sync file.");

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
    TimerWithEvent::init(base);
    NetFrameworkImplWithEvent::init(base);

    SyncCliLogic main_logic;
    LogicUdp &udplogic = main_logic;
    LogicTcp &tcplogic = main_logic;

    NetTrigger *discover_tr = new UdpTrigger(Trigger::second(2), true, udplogic, main_logic.getDiscoverLogic());

    main_logic.setDiscoveryTrigger(discover_tr);
    TimerWithEvent::addTr(discover_tr);

    vector<LocalPort> ports = LocalPort::query();

    for (size_t i = 0; i < ports.size(); i++)
    {
        LocalPort port = ports[i];
        auto broadaddr = port.getBroadAddr();
        NetAddr addr = NetAddr::fromBe(broadaddr);
        addr.setPort(atoi(ConfigManager::query(CONFIG_KEY_DISCOVER_SERVER_UDP_PORT).data()));
        main_logic.getDiscoveryTrigger().addConn(addr);
    }

    NetTrigger *sync_req_tr = new TcpTrigger(Trigger::second(5), true, tcplogic, main_logic.getSyncLogic());
    main_logic.setSyncTrigger(sync_req_tr);
    TimerWithEvent::addTr(sync_req_tr);

    TimerWithEvent::run();

    return 0;
}
