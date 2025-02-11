
#include <cstring>
#include <string>

#include "net/net_framework_impl_with_event.h"

int count = 0;

class UdpCli : public NetAbilityImplWithEvent
{
private:
public:
    UdpCli(NetAddr addr) : NetAbilityImplWithEvent(addr){};
    ~UdpCli();

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    uint64_t isExtraAllDataNow(void *data, uint64_t data_len) override;
};

UdpCli::~UdpCli()
{
}

void UdpCli::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    if (count >= 5)
    {
        NetFrameworkImplWithEvent::shutdown();
        return;
    }

    char *str = (char *)data;
    printf("recv:[%s]\n", str);
    ctx->write(data, data_len);
    count++;
}
uint64_t UdpCli::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return data_len;
}

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(*base);

    NetworkConnCtx *ctx = NetFrameworkImplWithEvent::connectWithUdp(new UdpCli(NetAddr("127.0.0.1:8080")));
    if (ctx != nullptr)
    {
        std::string msg = "hello world";
        ctx->write(msg.data(), msg.size());
    }

    NetFrameworkImplWithEvent::run();
    NetFrameworkImplWithEvent::free();

    return 0;
}
