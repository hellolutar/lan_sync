
#include <cstring>

#include "net/network_layer_with_event.h"

class TcpServer : public NetAbilityImplWithEvent
{
private:
public:
    TcpServer(struct sockaddr_in addr) : NetAbilityImplWithEvent(addr){};
    ~TcpServer();

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
};

TcpServer::~TcpServer()
{
}

void TcpServer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    char *str = (char *)data;
    printf("recv:[%s]\n", str);
    ctx->write(data, data_len);
}
bool TcpServer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return true;
}

int main(int argc, char const *argv[])
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    NetFrameworkImplWithEvent::addTcpServer(new TcpServer(addr));
    NetFrameworkImplWithEvent::run();
    NetFrameworkImplWithEvent::free();

    return 0;
}
