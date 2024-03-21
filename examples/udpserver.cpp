#include <cstring>
#include <string>

#include "net/net_framework_impl_with_event.h"

using namespace std;

class UdpServer : public NetAbilityImplWithEvent
{
private:
public:
    UdpServer(NetAddr addr) : NetAbilityImplWithEvent(addr){};
    ~UdpServer();

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    void isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len);
};

UdpServer::~UdpServer()
{
}

void UdpServer::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
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
void UdpServer::isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len)
{
    want_to_extra_len = data_len;
}

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(*base);

    NetFrameworkImplWithEvent::addUdpServer(new UdpServer(NetAddr(":58080")));
    NetFrameworkImplWithEvent::run();
    NetFrameworkImplWithEvent::free();

    return 0;
}
