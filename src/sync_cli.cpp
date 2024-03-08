#include "sync_cli.h"

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    TimerWithEvent::init(base);

    SyncCliLogic logic;
    SyncCliLogicUdp udplogic = logic;
    SyncCliLogicTcp tcplogic = logic;

    NetCliConnDiscover discover_conn;
    NetTrigger *discover_tr = new NetTrigger(Trigger::second(2), true, udplogic, discover_conn);
    logic.setDiscoveryTrigger(discover_tr);
    TimerWithEvent::addTr(discover_tr);

    vector<LocalPort> ports = LocalPort::query();

    for (size_t i = 0; i < ports.size(); i++)
    {
        LocalPort port = ports[i];
        auto broadaddr = port.getBroadAddr();
        NetAddr addr = NetAddr::fromBe(broadaddr);
        addr.setPort(DISCOVER_SERVER_UDP_PORT);
        logic.getDiscoveryTrigger().addNetAddr(addr);
    }

    NetCliConnReqTableIndex reqidx_conn;
    NetTrigger *reqidx_tr = new NetTrigger(Trigger::second(5), true, tcplogic, reqidx_conn);
    logic.setReqTableIndexTrigger(reqidx_tr);
    TimerWithEvent::addTr(reqidx_tr);


    TimerWithEvent::run();

    return 0;
}
