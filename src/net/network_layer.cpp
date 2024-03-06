#include "network_layer.h"
#include <stdlib.h>
#include <unistd.h>

NetworkLayer::~NetworkLayer()
{
}

NetworkEndpoint::~NetworkEndpoint()
{
    if (sock > 0)
        close(sock);

    free(addr);
}

void NetworkEndpoint::setSock(int sock)
{
    this->sock = sock;
}

struct sockaddr_in *NetworkEndpoint::getAddr()
{
    return addr;
}

NetworkConnCtx::~NetworkConnCtx()
{
}

NetworkEndpoint *NetworkConnCtx::getNetworkEndpoint()
{
    return ne;
}

void NetworkConnCtx::destroy()
{
    for (auto iter = head->end() - 1; iter >= head->begin(); iter--)
    {
        if (*iter == this)
        {
            head->erase(iter);
            break;
        }
    }
    delete this;
}
