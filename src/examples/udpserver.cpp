#include <cstring>

#include "net/network_layer_with_event.h"

class UdpServer : public NetworkEndpointWithEvent
{
private:
public:
    UdpServer(struct sockaddr_in *addr) : NetworkEndpointWithEvent(addr){};
    ~UdpServer();

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
};

UdpServer::~UdpServer()
{
}

void UdpServer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    char *str = (char *)data;
    printf("recv:[%s]\n", str);
    ctx->write(data, data_len);
}
bool UdpServer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return true;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in sock;
    sock.sin_family = AF_INET;
    sock.sin_port = htons(8080);
    sock.sin_addr.s_addr = htonl(INADDR_ANY);

    NetworkLayerWithEvent::addUdpServer(new UdpServer(&sock));
    NetworkLayerWithEvent::run();
    NetworkLayerWithEvent::free();

    return 0;
}
