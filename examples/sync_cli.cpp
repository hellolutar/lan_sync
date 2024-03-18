#include "main.h"

int main(int argc, char const *argv[])
{
    load_config(argc, argv);

    struct event_base *base = event_base_new();
    TimerWithEvent::init(base);
    NetFrameworkImplWithEvent::init(*base);

    SyncCliLogic main_logic;
    LogicUdp &udplogic = main_logic;
    LogicTcp &tcplogic = main_logic;

    NetTrigger *discover_tr = new UdpTrigger(Trigger::second(2), true, udplogic, main_logic.getDiscoverLogic());

    main_logic.setDiscoveryTrigger(discover_tr);
    TimerWithEvent::addTr(discover_tr);

    vector<LocalPort> ports = LocalPort::query();

    for (int i = 0; i < ports.size(); i++)
    {
        LocalPort port = ports[i];
        auto broadaddr = port.getBroadAddr();
        NetAddr addr = NetAddr::fromBe(broadaddr);
        addr.setPort(atoi(ConfigManager::query(CONFIG_KEY_DISCOVER_SERVER_UDP_PORT).data()));
        main_logic.getDiscoveryTrigger().addConn(addr);
    }

    vector<string> ip = ConfigManager::queryList(CONFIG_KEY_DISCOVER_IPS);
    for (int i = 0; i < ip.size(); i++)
    {
        NetAddr addr(ip[i]);
        if (addr.getPort() == 0)
        {
            addr.setPort(atoi(ConfigManager::query(CONFIG_KEY_DISCOVER_SERVER_UDP_PORT).data()));
        }
        main_logic.getDiscoveryTrigger().addConn(addr);
    }

    NetTrigger *sync_req_tr = new TcpTrigger(Trigger::second(2), true, tcplogic, main_logic.getSyncLogic());
    main_logic.setSyncTrigger(sync_req_tr);
    TimerWithEvent::addTr(sync_req_tr);

    event_base_dispatch(base);
    event_base_free(base);

    return 0;
}
