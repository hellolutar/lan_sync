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

NetworkContext::~NetworkContext()
{
}

NetworkEndpoint *NetworkContext::getNetworkEndpoint()
{
    return ne;
}

NetworkOutputStream::~NetworkOutputStream()
{
}

NetworkOutputStreamForUDP::~NetworkOutputStreamForUDP()
{
    if (sock > 0)
        close(sock);
}

uint64_t NetworkOutputStreamForUDP::write(void *data, uint64_t data_len)
{
    return sendto(sock, data, data_len, 0, (sockaddr *)&peer, sizeof(sockaddr_in));
}
