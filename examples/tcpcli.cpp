
#include <cstring>
#include <string>

#include "net/net_framework_impl_with_event.h"

int count = 0;

class TcpCli : public NetAbilityImplWithEvent
{
private:
public:
    TcpCli(NetAddr addr) : NetAbilityImplWithEvent(addr){};
    ~TcpCli();

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx) override;
    uint64_t isExtraAllDataNow(void *data, uint64_t data_len) override;
};

TcpCli::~TcpCli()
{
}

void TcpCli::recv(void *data, uint64_t data_len, NetworkConnCtx *ctx)
{
    if (count >= 5)
    {
        printf("Done\n");
        NetFrameworkImplWithEvent::shutdown();
        return;
    }

    char *str = (char *)data;
    printf("recv:[%s]\n", str);
    ctx->write(data, data_len);
    count++;
}
uint64_t TcpCli::isExtraAllDataNow(void *data, uint64_t data_len)
{
    return data_len;
}

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(*base);

    NetworkConnCtx *ctx = NetFrameworkImplWithEvent::connectWithTcp(new TcpCli(NetAddr("127.0.0.1:8080")));
    if (ctx != nullptr)
    {
        std::string msg = "hello world";
        ctx->write(msg.data(), msg.size());
    }

    NetFrameworkImplWithEvent::run();
    NetFrameworkImplWithEvent::free();
    event_base_free(base);

    return 0;
}
