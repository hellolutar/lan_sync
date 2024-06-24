#include <cstring>
#include <string>

#include <event2/thread.h>

#include "net/net_framework_impl_with_event.h"

using namespace std;

class UdpServer : public NetAbilityImplWithEvent
{
private:
public:
    UdpServer(NetAddr addr) : NetAbilityImplWithEvent(addr){};
    ~UdpServer();

    void recv(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx) override;
    uint64_t isExtraAllDataNow(void *data, uint64_t data_len);
};

UdpServer::~UdpServer()
{
}

void UdpServer::recv(void *data, uint64_t data_len, std::shared_ptr<NetworkConnCtx> ctx)
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
uint64_t UdpServer::isExtraAllDataNow(void *data, uint64_t data_len)
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

    NetFrameworkImplWithEvent::addUdpServer(new UdpServer(NetAddr(":58080")));
    NetFrameworkImplWithEvent::run();
    NetFrameworkImplWithEvent::free();

    return 0;
}
