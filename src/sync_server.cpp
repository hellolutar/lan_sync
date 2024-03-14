#include "sync_server.h"

int main(int argc, char const *argv[])
{
    configure(argc, argv);

    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(*base);

    SyncSrvLogic main_logic;
    LogicTcp &tcplogic = main_logic;
    LogicUdp &udplogic = main_logic;

    NetFrameworkImplWithEvent::addUdpServer(new NetSrvLogicContainer(NetAddr(":58080"), udplogic));
    NetFrameworkImplWithEvent::addTcpServer(new NetSrvLogicContainer(NetAddr(":58081"), tcplogic));

    event_base_dispatch(base);
    event_base_free(base);

    NetFrameworkImplWithEvent::free();

    return 0;
}