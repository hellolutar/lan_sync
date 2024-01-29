
#include "discover.h"

Discover::Discover(/* args */)
{
    st = DISCOVERING;
}

Discover::~Discover()
{
}

int getLoclInfo(struct local_inf_info *infos)
{
    int fd;
    assert((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0);

    struct ifreq ifreqs[10];

    struct ifconf ifc;
    ifc.ifc_len = sizeof(ifreqs);
    ifc.ifc_buf = (caddr_t)ifreqs;

    assert(ioctl(fd, SIOCGIFCONF, (char *)&ifc) >= 0);
    int interface_num = ifc.ifc_len / sizeof(struct ifreq);

    for (size_t i = 0; i < interface_num; i++)
    {
        memcpy(infos[i].name, ifreqs[i].ifr_name, strlen(ifreqs[i].ifr_name));

        // ip
        assert(ioctl(fd, SIOCGIFADDR, &ifreqs[i]) >= 0); // 获取ip，并将其存储到ifreq中
        infos[i].addr = *(struct sockaddr_in *)&(ifreqs[i].ifr_addr);

        // // broad address of this interface
        assert(ioctl(fd, SIOCGIFBRDADDR, &ifreqs[i]) >= 0); // 获取广播地址，并将其存储到ifreq中
        infos[i].broad_addr = *(struct sockaddr_in *)&(ifreqs[i].ifr_broadaddr);

        // // subnet mask
        assert(ioctl(fd, SIOCGIFNETMASK, &ifreqs[i]) >= 0);
        infos[i].subnet_mask = *(struct sockaddr_in *)&(ifreqs[i].ifr_netmask);
    }

    return interface_num;
}

void configure_sock(sockaddr_in *const target_sock_addr, const struct local_inf_info &info)
{
    printf("want to send udp to %s \n", inet_ntoa(info.broad_addr.sin_addr));
    target_sock_addr->sin_family = AF_INET;
    target_sock_addr->sin_addr = info.broad_addr.sin_addr;
    target_sock_addr->sin_port = htons(DISCOVER_SERVER_UDP_PORT);
}

int send_to(int sock, const sockaddr_in &target_sock_addr, string data)
{
    sdn_share_proto_header_t tmp_header = {v0_1, HELLO, (uint16_t)data.size()};
    sdn_share_proto_header_t *header = (sdn_share_proto_header_t *)encapsulate(tmp_header, data.data(), data.size());

    int ret = sendto(sock, header, sizeof(header) + data.size(), 0, (struct sockaddr *)&target_sock_addr, sizeof(target_sock_addr));
    free(header);
    printf("udp send result :%d \n", ret);
    return ret;
}

void *thread_cb(void *arg)
{
    // todo
    return nullptr;
}

bool Discover::handle_hello_recv(int sock, const sockaddr_in &target_sock_addr)
{
    char buf[4096] = {0};
    int addr_len = sizeof(target_sock_addr);
    int len = recvfrom(sock, buf, 4096, 0, (struct sockaddr *)&target_sock_addr, (socklen_t *)&addr_len);

    if (len > 0)
    {
        struct sdn_share_proto_header *header = (sdn_share_proto_header_t *)buf;
        debug_print_header(header, target_sock_addr.sin_addr);

        // 这里可以适用设计模式
        if (header->type == HELLO_ACK)
        {
            // 启动HTTPS Server
            if (this->st == DISCOVERING)
            {
                // todo
                pthread_t tid;
                pthread_create(&tid, nullptr, thread_cb, nullptr);
            }
            this->st = SYNC_READY;
            return true;
        }
        else
            printf("%s : %d", "unsupport type, do not reply \n", header->type);
    }
    return false;
}

void Discover::start()
{
    struct local_inf_info infos[10];
    memset(infos, 0, sizeof(struct local_inf_info) * 10);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock > 0);

    int optval = 1; // 这个值一定要设置，否则可能导致sendto()失败
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));

    for (size_t i = 0; i < getLoclInfo(infos); i++)
    {
        struct local_inf_info info = infos[i];

        if (info.broad_addr.sin_addr.s_addr == 0)
            continue;

        sockaddr_in target_sock_addr;
        configure_sock(&target_sock_addr, info);

        int ret = send_to(sock, target_sock_addr, "hello");
        if (ret < 0)
            continue;
        if (handle_hello_recv(sock, target_sock_addr))
            break;
    }
}

int main(int argc, char const *argv[])
{
    Discover discover;
    discover.start();
    return 0;
}
