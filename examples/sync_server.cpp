#include "main.h"

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
    mediator.add(&sync_srv_logic);

    int ret = evthread_use_pthreads();
    if (ret != 0)
    {
        printf("unsupport evthread_use_pthreads()\n");
        return -1;
    }
    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(*base);
    TimerWithEvent::init(base);

    configSyncSrv(sync_srv_logic);

    event_base_dispatch(base);
    event_base_free(base);

    NetFrameworkImplWithEvent::free();
    return 0;
}