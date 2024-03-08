#include "sync_cli.h"

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();

    SyncCliLogic logic;
    SyncCliLogicUdp udplogic = logic;
    SyncCliLogicTcp tcplogic = logic;

    NetCliConnDiscover discover_conn;
    NetTrigger *discover_tr = new NetTrigger(Trigger::second(2), true, udplogic, discover_conn);
    logic.setDiscoveryTrigger(discover_tr);

    vector<LocalPort> ports = LocalPort::query();

    for (size_t i = 0; i < ports.size(); i++)
    {
        LocalPort port = ports[i];
        auto broadaddr = port.getBroadAddr();
        broadaddr.sin_port = htons(DISCOVER_SERVER_UDP_PORT);
        NetAddr addr = NetAddr::fromBe(broadaddr);
        logic.discovery->addNetAddr(addr);
    }

    TimerWithEvent::init(base);
    TimerWithEvent::addTr(discover_tr);
    TimerWithEvent::run();

    return 0;
}
