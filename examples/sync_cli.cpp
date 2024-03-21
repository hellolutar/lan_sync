#include "main.h"

SyncCliLogic sync_cli_logic;

void configSyncCli()
{
    LOG_INFO("configSyncCli()...");
    LogicUdp &udplogic = sync_cli_logic;
    LogicTcp &tcplogic = sync_cli_logic;

    NetTrigger *discover_tr = new UdpTrigger(Trigger::second(2), true, udplogic, sync_cli_logic.getDiscoverLogic());

    sync_cli_logic.setDiscoveryTrigger(discover_tr);
    TimerWithEvent::addTr(discover_tr);

    vector<LocalPort> ports = LocalPort::query();
    for (int i = 0; i < ports.size(); i++)
    {
        LocalPort port = ports[i];
        auto broadaddr = port.getBroadAddr();
        NetAddr addr = NetAddr::fromBe(broadaddr);
        addr.setPort(atoi(ConfigManager::query(CONFIG_KEY_PROTO_DISCOVER_SERVER_UDP_PORT).data()));
        sync_cli_logic.getDiscoveryTrigger().addConn(addr);
    }

    vector<string> ip = ConfigManager::queryList(CONFIG_KEY_DISCOVER_IPS);
    for (int i = 0; i < ip.size(); i++)
    {
        NetAddr addr(ip[i]);
        if (addr.getPort() == 0)
        {
            addr.setPort(atoi(ConfigManager::query(CONFIG_KEY_PROTO_DISCOVER_SERVER_UDP_PORT).data()));
        }
        sync_cli_logic.getDiscoveryTrigger().addConn(addr);
    }

    NetTrigger *sync_req_tr = new TcpTrigger(Trigger::second(2), true, tcplogic, sync_cli_logic.getSyncLogic());
    sync_cli_logic.setSyncTrigger(sync_req_tr);
    TimerWithEvent::addTr(sync_req_tr);
}

int main(int argc, char const *argv[])
{
    configlog(spdlog::level::debug);
    load_config(argc, argv);

    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(*base);
    TimerWithEvent::init(base);

    configSyncCli();

    event_base_dispatch(base);
    event_base_free(base);

    NetFrameworkImplWithEvent::free();
    return 0;
}