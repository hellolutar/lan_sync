#include <cstring>

#include "comm/network_layer_tcp_with_event.h"

class Udp : public NetworkEndpoint
{
private:
public:
    Udp(struct sockaddr_in *addr) : NetworkEndpoint(addr){};
    ~Udp();

    void recv(void *data, uint64_t data_len, NetworkContext *ctx);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
};

Udp::~Udp()
{
}

void Udp::recv(void *data, uint64_t data_len, NetworkContext *ctx)
{
    char *str = (char *)data;
    printf("recv:[%s]\n", str);
    ctx->write(data, data_len);
}
bool Udp::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return true;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in sock;
    sock.sin_family = AF_INET;
    sock.sin_port = htons(8080);
    sock.sin_addr.s_addr = htonl(INADDR_ANY);

    NetworkLayerWithEvent::addUdpServer(new Udp(&sock));
    NetworkLayerWithEvent::run();

    return 0;
}
