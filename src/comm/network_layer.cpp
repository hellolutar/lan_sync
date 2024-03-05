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
