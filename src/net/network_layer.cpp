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
    if (peer!=nullptr)
        delete peer;
}

NetworkEndpoint *NetworkConnCtx::getNetworkEndpoint()
{
    return ne;
}

NetAddr& NetworkConnCtx::getPeer(){
    return *peer;
}
void NetworkConnCtx::setNetAddr(NetAddr *peer){
    this->peer = peer;
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
