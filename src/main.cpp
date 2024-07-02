#include "main.h"

int main(int argc, char const *argv[])
{
    load_config(argc, argv);
    configlog(ConfigManager::query(CONFIG_KEY_LOG_LEVEL));

    int ret = evthread_use_pthreads();
    if (ret != 0)
    {
        printf("unsupport evthread_use_pthreads()\n");
        return -1;
    }
    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(*base);
    TimerWithEvent::init(base);

    SyncModConnMediator mediator;

    shared_ptr<TaskManager2> tmp = make_shared<TaskManager2>();
    TaskCoordinator coor(tmp);
    TaskCoordinatorTrigger trg(Trigger::second(10), true, coor);
    TaskCoordinatorTriggerModconn coorTrgMod(&mediator, trg);

    TimerWithEvent::addTr(&trg);

    mediator.add(&coorTrgMod);

    SyncService service(&mediator, coor);
    SyncController cntrl(service);

    LogicUdp &udplogic = cntrl;
    LogicTcp &tcplogic = cntrl;

    SyncCliDiscoverLogic discover_logic;

    DiscoveryTrigger discover_tr(Trigger::second(2), true, udplogic, &mediator);
    TimerWithEvent::addTr(&discover_tr);

    vector<LocalPort> ports = LocalPort::query();
    for (int i = 0; i < ports.size(); i++)
    {
        LocalPort port = ports[i];
        auto broadaddr = port.getBroadAddr();
        NetAddr addr = NetAddr::fromBe(broadaddr);
        addr.setPort(atoi(ConfigManager::query(CONFIG_KEY_PROTO_DISCOVER_SERVER_UDP_PORT).data()));
        discover_tr.addConn(addr);
    }

    vector<string> ip = ConfigManager::queryList(CONFIG_KEY_DISCOVER_IPS);
    for (int i = 0; i < ip.size(); i++)
    {
        NetAddr addr(ip[i]);
        if (addr.getPort() == 0)
        {
            addr.setPort(atoi(ConfigManager::query(CONFIG_KEY_PROTO_DISCOVER_SERVER_UDP_PORT).data()));
        }
        discover_tr.addConn(addr);
    }

    mediator.add(&service);
    mediator.add(&discover_tr);

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

    event_base_dispatch(base);
    event_base_free(base);

    NetFrameworkImplWithEvent::free();
    return 0;
}