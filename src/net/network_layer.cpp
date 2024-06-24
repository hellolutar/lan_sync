#include "network_layer.h"
#include <stdlib.h>
#include <unistd.h>

AbstNetFramework::~AbstNetFramework()
{
}

NetAbility::~NetAbility()
{
    if (sock > 0)
        close(sock);
}

void NetAbility::setSock(int sock)
{
    this->sock = sock;
}

struct NetAddr &NetAbility::getAddr()
{
    return addr;
}

NetworkConnCtx::~NetworkConnCtx()
{
    for (auto iter = head->end() - 1; iter >= head->begin(); iter--)
    {
        if ((*iter).get() == this)
        {
            head->erase(iter);
            break;
        }
    }
}

NetAbility *NetworkConnCtx::getNetworkEndpoint()
{
    return ne;
}

NetAddr &NetworkConnCtx::getPeer()
{
    return peer;
}
void NetworkConnCtx::setNetAddr(NetAddr peer)
{
    this->peer = peer;
}

bool NetworkConnCtx::isActive()
{
    return active;
}

void NetworkConnCtx::setActive(bool status)
{
    active = status;
}