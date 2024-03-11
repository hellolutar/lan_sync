
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

    void recv(void *data, uint64_t data_len, NetworkConnCtx *ctx);
    void isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len);
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
void TcpCli::isExtraAllDataNow(void *data, uint64_t data_len, uint64_t &want_to_extra_len)
{
    want_to_extra_len = data_len;
}

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    NetFrameworkImplWithEvent::init(base);

    NetworkConnCtx *ctx = NetFrameworkImplWithEvent::connectWithTcp(new TcpCli(NetAddr("127.0.0.1:8080")));
    if (ctx != nullptr)
    {
        std::string msg = "hello world";
        ctx->write(msg.data(), msg.size());
    }

    NetFrameworkImplWithEvent::run();
    NetFrameworkImplWithEvent::free();

    return 0;
}
