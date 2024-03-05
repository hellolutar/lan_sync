#include <cstring>

#include "comm/network_layer_tcp_with_event.h"

class TcpServer : public NetworkEndpoint
{
private:
public:
    TcpServer(struct sockaddr_in *addr) : NetworkEndpoint(addr){};
    ~TcpServer();

    void recv(void *data, uint64_t data_len);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
};

TcpServer::~TcpServer()
{
}

void TcpServer::recv(void *data, uint64_t data_len)
{
    char *str = (char *)data;
    printf("recv: %s \n", str);

    NetworkLayerWithEvent::send(this, data, data_len);
}
bool TcpServer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return true;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in sock;
    sock.sin_family = AF_INET;
    sock.sin_port = htons(8080);
    sock.sin_addr.s_addr = htonl(INADDR_ANY);

    NetworkLayerWithEvent::addTcpServer(new TcpServer(&sock));
    NetworkLayerWithEvent::run();

    return 0;
}
