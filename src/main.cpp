#include "main.h"

void configSyncCli(SyncCliLogic &sync_cli_logic)
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

    NetTrigger *sync_req_tr = new TcpTrigger(Trigger::second(10), true, tcplogic, sync_cli_logic.getSyncLogic());
    sync_cli_logic.setSyncTrigger(sync_req_tr);
    TimerWithEvent::addTr(sync_req_tr);
}

void configSyncSrv(SyncSrvLogic &sync_srv_logic)
{
    LOG_INFO("configSyncSrv()...");

    LogicTcp &tcplogic = sync_srv_logic;
    LogicUdp &udplogic = sync_srv_logic;

    string listen_udp = StringBuilder::builder()
                            .add(":")
                            .add(ConfigManager::query(CONFIG_KEY_PROTO_DISCOVER_SERVER_UDP_PORT))
                            .str();
    string listen_tcp = StringBuilder::builder()
                            .add(":")
                            .add(ConfigManager::query(CONFIG_KEY_PROTO_SYNC_SERVER_TCP_PORT))
                            .str();

    NetFrameworkImplWithEvent::addUdpServer(new NetSrvLogicContainer(listen_udp, udplogic));
    NetFrameworkImplWithEvent::addTcpServer(new NetSrvLogicContainer(listen_tcp, tcplogic));
}

int main(int argc, char const *argv[])
{
    load_config(argc, argv);
    configlog(ConfigManager::query(CONFIG_KEY_LOG_LEVEL));

    SyncModConnMediator mediator;
    SyncSrvLogic sync_srv_logic(mediator, MODULE_NAME_SYNC_SRV);
    SyncCliLogic sync_cli_logic(mediator, MODULE_NAME_SYNC_CLI);
    mediator.add(&sync_srv_logic);
    mediator.add(&sync_cli_logic);

    int ret = evthread_use_pthreads();
    if (ret != 0)
    {
        printf("unsupport evthread_use_pthreads()\n");
        return -1;
    }
    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(*base);
    TimerWithEvent::init(base);

    configSyncCli(sync_cli_logic);
    configSyncSrv(sync_srv_logic);

    event_base_dispatch(base);
    event_base_free(base);

    NetFrameworkImplWithEvent::free();
    return 0;
}