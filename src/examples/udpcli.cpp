
#include <cstring>
#include <string>

#include "net/network_layer_with_event.h"

int count = 0;

class UdpCli : public NetworkEndpoint
{
private:
public:
    UdpCli(struct sockaddr_in *addr) : NetworkEndpoint(addr){};
    ~UdpCli();

    void recv(void *data, uint64_t data_len, NetworkContext *ctx);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
};

UdpCli::~UdpCli()
{
}

void UdpCli::recv(void *data, uint64_t data_len, NetworkContext *ctx)
{
    if (count >= 5)
    {
        NetworkLayerWithEvent::shutdown();
        return;
    }

    char *str = (char *)data;
    printf("recv:[%s]\n", str);
    ctx->write(data, data_len);
    count++;
}
bool UdpCli::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return true;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(8080);
    inet_aton("127.0.0.1", &(peer.sin_addr));

    NetworkOutputStream *out = NetworkLayerWithEvent::connectWithUdp(new UdpCli(&peer));
    if (out != nullptr)
    {
        std::string msg = "hello world";
        out->write(msg.data(), msg.size());
    }

    return 0;
}
