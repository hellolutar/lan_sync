#include "sync_cli.h"

SyncCli::SyncCli()
{
}

void SyncCli::setDiscoveryTrigger(DiscoveryTrigger *tr)
{
    discovery = tr;
}

SyncCli::~SyncCli()
{
    delete discovery;
}

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    SyncCli cli;

    cli.setDiscoveryTrigger(new DiscoveryTrigger(Trigger::second(2), true));

    vector<LocalPort> ports = LocalPort::query();

    for (size_t i = 0; i < ports.size(); i++)
    {
        LocalPort port = ports[i];
        NetAddr addr = NetAddr::fromBe(port.getBroadAddr());
        cli.discovery->addNetAddr(addr);
    }

    TimerWithEvent::init(base);
    TimerWithEvent::addTr(cli.discovery);
    TimerWithEvent::run();

    return 0;
}
