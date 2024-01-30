#include "discover_server.h"

DiscoverServer::DiscoverServer(/* args */)
{
    st = STATE_DISCOVERING;
}

DiscoverServer::~DiscoverServer()
{
}

void *thread_cb(void *arg)
{
    // todo
    return nullptr;
}

void DiscoverServer::start()
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    assert(fd > 0);

    struct sockaddr_in addr;
    addr.sin_port = htons(DISCOVER_SERVER_UDP_PORT);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(fd, 10);
    printf("listen: %d\n", DISCOVER_SERVER_UDP_PORT);

    while (1)
    {
        struct sockaddr_in cli_addr;
        char buf[4096] = {0};
        int addr_len = sizeof(cli_addr);
        int len = recvfrom(fd, buf, 4096, 0, (struct sockaddr *)&cli_addr, (socklen_t *)&addr_len);

        if (len > 0)
        {
            struct lan_discover_header *header = (lan_discover_header_t *)buf;
            debug_print_header(header, cli_addr.sin_addr);

            // 这里可以适用设计模式
            if (header->type == LAN_DISCOVER_TYPE_HELLO)
            {
                // 启动HTTPS Server
                if (this->st == STATE_DISCOVERING)
                {
                    // todo 开线程，建立HTTP服务器
                    pthread_t other_thread_id;
                    pthread_create(&other_thread_id, nullptr, thread_cb, nullptr);
                }
                this->st = STATE_SYNC_READY;
                uint16_t msg = DISCOVER_SERVER_HTTP_PORT;

                lan_discover_header_t reply_header = {LAN_DISCOVER_VER_0_1, LAN_DISCOVER_TYPE_HELLO_ACK, sizeof(msg)};
                lan_discover_header_t *header = (lan_discover_header_t *)encapsulate(reply_header, &msg, sizeof(msg));
                int ret = sendto(fd, header, sizeof(header) + sizeof(msg), 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
                free(header);
                assert(ret > 0);
            }
            else
                printf("%s : %d", "unsupport type, do not reply \n", header->type);
        }
    }
}

int main(int argc, char const *argv[])
{
    DiscoverServer disconverServer;
    disconverServer.start();
    return 0;
}
