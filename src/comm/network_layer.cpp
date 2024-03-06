#include "network_layer.h"
#include <stdlib.h>

NetworkLayer::~NetworkLayer()
{
}

NetworkEndpoint::~NetworkEndpoint()
{
    free(addr);
}

struct sockaddr_in *NetworkEndpoint::getAddr()
{
    return addr;
}

NetworkContext::~NetworkContext()
{
}

NetworkEndpoint *NetworkContext::getNetworkEndpoint()
{
    return ne;
}