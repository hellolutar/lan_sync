
#include <cstring>
#include <string>

#include "net/net_framework_impl_with_event.h"

using namespace std;

class TcpServer : public NetAbilityImplWithEvent
{
private:
public:
    TcpServer(NetAddr addr) : NetAbilityImplWithEvent(addr){};
    ~TcpServer();

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
};

TcpServer::~TcpServer()
{
}

void TcpServer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    string msg((char *)data);
    printf("recv:[%s]\n", msg.data());
    size_t pos = msg.find("exit");
    if (pos != string::npos)
    {
        NetFrameworkImplWithEvent::shutdown();
        return;
    }

    ctx->write(data, data_len);
}
bool TcpServer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return true;
}

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(base);

    NetFrameworkImplWithEvent::addTcpServer(new TcpServer(NetAddr(":8080")));
    NetFrameworkImplWithEvent::run();
    NetFrameworkImplWithEvent::free();

    return 0;
}
