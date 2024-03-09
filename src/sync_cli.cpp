#include "sync_cli.h"

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    TimerWithEvent::init(base);
    NetFrameworkImplWithEvent::init(base);

    SyncCliLogic main_logic;
    SyncCliLogicUdp udplogic = main_logic;
    SyncCliLogicTcp tcplogic = main_logic;

    NetTrigger *discover_tr = new UdpTrigger(Trigger::second(2), true, udplogic, main_logic.getDiscoverLogic());

    main_logic.setDiscoveryTrigger(discover_tr);
    TimerWithEvent::addTr(discover_tr);

    vector<LocalPort> ports = LocalPort::query();

    for (size_t i = 0; i < ports.size(); i++)
    {
        LocalPort port = ports[i];
        auto broadaddr = port.getBroadAddr();
        NetAddr addr = NetAddr::fromBe(broadaddr);
        addr.setPort(DISCOVER_SERVER_UDP_PORT);
        main_logic.getDiscoveryTrigger().addNetAddr(addr);
    }

    NetTrigger *reqidx_tr = new TcpTrigger(Trigger::second(5), true, tcplogic, main_logic.getSyncCliReqTbIdxLogic());
    main_logic.setReqTableIndexTrigger(reqidx_tr);
    TimerWithEvent::addTr(reqidx_tr);

    TimerWithEvent::run();

    return 0;
}
