
#include <cstring>
#include <string>

#include <event2/thread.h>

#include "net/net_framework_impl_with_event.h"

using namespace std;

class TcpServer : public NetAbilityImplWithEvent
{
private:
public:
    TcpServer(NetAddr addr) : NetAbilityImplWithEvent(addr){};
    ~TcpServer();

    void recv(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx) override;
    uint64_t isExtraAllDataNow(void *data, uint64_t data_len) override;
};

TcpServer::~TcpServer()
{
}

void TcpServer::recv(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx)
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
uint64_t TcpServer::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return data_len;
}

int main(int argc, char const *argv[])
{
    int ret = evthread_use_pthreads();
    if (ret != 0)
    {
        printf("unsupport evthread_use_pthreads()\n");
        return -1;
    }

    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(*base);

    NetFrameworkImplWithEvent::addTcpServer(new TcpServer(NetAddr(":8080")));
    NetFrameworkImplWithEvent::run();
    NetFrameworkImplWithEvent::free();

    return 0;
}
