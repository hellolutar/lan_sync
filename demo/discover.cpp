
#include "discover.h"

#include <event2/buffer.h>
#include <event2/bufferevent.h>

struct event_base *base = event_base_new();
Discover *discover = new Discover(base);

int getLoclInfo(struct local_inf_info *inf_infos)
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
        memcpy(inf_infos[i].name, ifreqs[i].ifr_name, strlen(ifreqs[i].ifr_name));

        // ip
        assert(ioctl(fd, SIOCGIFADDR, &ifreqs[i]) >= 0); // 获取ip，并将其存储到ifreq中
        inf_infos[i].addr = *(struct sockaddr_in *)&(ifreqs[i].ifr_addr);

        // // broad address of this interface
        assert(ioctl(fd, SIOCGIFBRDADDR, &ifreqs[i]) >= 0); // 获取广播地址，并将其存储到ifreq中
        inf_infos[i].broad_addr = *(struct sockaddr_in *)&(ifreqs[i].ifr_broadaddr);

        // // subnet mask
        assert(ioctl(fd, SIOCGIFNETMASK, &ifreqs[i]) >= 0);
        inf_infos[i].subnet_mask = *(struct sockaddr_in *)&(ifreqs[i].ifr_netmask);
    }

    return interface_num;
}

Discover::Discover(struct event_base *base)
{
    assert(base);

    this->st = STATE_DISCOVERING;
    this->base = base;
    this->inf_infos = (struct local_inf_info *)malloc(sizeof(struct local_inf_info) * 10);
    memset(inf_infos, 0, sizeof(struct local_inf_info) * 10);
    this->inf_infos_len = getLoclInfo(inf_infos);

    this->target_sock_addrs = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in) * inf_infos_len);
    memset(target_sock_addrs, 0, sizeof(struct sockaddr_in) * inf_infos_len);
}

Discover::~Discover()
{
    if (inf_infos)
        free(inf_infos);
    if (target_sock_addrs)
        free(target_sock_addrs);
    inf_infos = nullptr;
    target_sock_addrs = nullptr;
}

void configure_sock(sockaddr_in *const target_sock_addr, const struct local_inf_info &info)
{
    target_sock_addr->sin_family = AF_INET;
    target_sock_addr->sin_addr = info.broad_addr.sin_addr;
    target_sock_addr->sin_port = htons(DISCOVER_SERVER_UDP_PORT);
}

void tcp_readcb(struct bufferevent *bev, void *ctx)
{
    printf("receive tcp msg! \n");
}

void tcp_writecb(struct bufferevent *bev, void *ctx)
{
    printf("can write msg to peer tcp server! \n");
}

void connect_http_server(struct cb_arg *arg)
{
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;

    int tcpsock = socket(AF_INET, SOCK_STREAM, 0);
    int ret = connect(tcpsock, (struct sockaddr *)arg->target_addr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        printf("[DEBUG] TCP ERROR: %s \n", strerror(errno));
        libevent_global_shutdown();
    }

    struct bufferevent *bev = bufferevent_socket_new(base, tcpsock, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, tcp_readcb, tcp_writecb, nullptr, base);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
    cb_arg_free(arg);
}

void udp_readcb(evutil_socket_t fd, short events, void *ctx)
{
    struct event_base *base = (struct event_base *)ctx;

    struct sockaddr_in target_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    char data[4096] = {0};

    int receive = recvfrom(fd, data, 4096, 0, (struct sockaddr *)&target_addr, &addrlen);
    if (receive <= 0)
    {
        printf("warning: cannot receive anything !\n");
        return;
    }

    struct lan_discover_header *header = (lan_discover_header_t *)data;
    uint16_t peer_tcp_port = *(uint16_t *)(header + 1);
    printf("recive msg, data_len:%d , port: %d\n", header->data_len, peer_tcp_port);

    if (header->type == LAN_DISCOVER_TYPE_HELLO_ACK)
    {
        // 启动HTTPS Server
        if (discover->st == STATE_DISCOVERING)
        {
            discover->st = STATE_SYNC_READY;

            // todo(lutar) 形成 RESOURCE_TABLE, 这里是不是也可以引入状态机

            struct cb_arg *arg = cb_arg_new(base);
            memcpy(arg->target_addr, &target_addr, sizeof(target_addr));
            arg->target_addr->sin_port = htons(peer_tcp_port);

            connect_http_server(arg);
            printf("[DEBUG] todo : add a tcp bufferevent to sync request!\n");
        }
    }
    else
        printf("%s : %d", "unsupport type, do not reply \n", header->type);
}

void udp_sent_hello(evutil_socket_t fd, short events, void *ctx)
{
    // todo (lutar) 引入定时器
    struct cb_arg *arg = (struct cb_arg *)ctx;

    string msg = "hello";
    lan_discover_header_t reply_header = {LAN_DISCOVER_VER_0_1, LAN_DISCOVER_TYPE_HELLO, sizeof(msg.data())};
    evbuffer_add(arg->buf, &reply_header, sizeof(lan_discover_header_t));
    evbuffer_add(arg->buf, msg.data(), msg.size());

    struct event *write_e = event_new(base, fd, EV_WRITE, writecb, arg);
    event_add(write_e, nullptr);
}

void Discover::start()
{
    struct event *write_event, *read_event;

    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(udp_sock > 0);

    int optval = 1; // 这个值一定要设置，否则可能导致sendto()失败
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    struct cb_arg *arg = cb_arg_new(base);

    for (size_t i = 0; i < inf_infos_len; i++)
    {
        struct local_inf_info info = inf_infos[i];

        if (info.broad_addr.sin_addr.s_addr == 0)
            continue;

        configure_sock(arg->target_addr, info);

        write_event = event_new(base, udp_sock, EV_WRITE, udp_sent_hello, arg); // todo(lutar) 实现周期发送
        assert(write_event);
        event_add(write_event, nullptr);

        break;
    }

    read_event = event_new(base, udp_sock, EV_READ | EV_PERSIST, udp_readcb, base); // todo(lutar) 实现周期发送
    assert(read_event);
    event_add(read_event, nullptr);

    event_base_dispatch(base);
    event_free(read_event); // todo(lutar): 这里应该不用释放
}

int main(int argc, char const *argv[])
{
    discover->start();
    event_base_free(base);
    free(discover);
    return 0;
}