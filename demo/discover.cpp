
#include "discover.h"

#include <event2/buffer.h>
#include <event2/bufferevent.h>

typedef struct cb_arg
{
    Discover dis;
    size_t index;
} cb_arg_t;

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

int send_to(int sock, const sockaddr_in &target_sock_addr, string data)
{
    lan_discover_header_t tmp_header = {LAN_DISCOVER_VER_0_1, LAN_DISCOVER_TYPE_HELLO, (uint16_t)data.size()};
    lan_discover_header_t *header = (lan_discover_header_t *)encapsulate(tmp_header, data.data(), data.size());

    int ret = sendto(sock, header, sizeof(header) + data.size(), 0, (struct sockaddr *)&target_sock_addr, sizeof(target_sock_addr));
    free(header);
    printf("udp send result :%d \n", ret);
    return ret;
}

bool Discover::handle_hello_recv(struct bufferevent *bev)
{
    struct evbuffer *input, *output;
    input = bufferevent_get_input(bev);

    char buf[evbuffer_get_length(input)] = {0};
    evbuffer_copyout(input, buf, evbuffer_get_length(input));

    struct lan_discover_header *header = (lan_discover_header_t *)buf;

    // 这里可以适用设计模式
    if (header->type == LAN_DISCOVER_TYPE_HELLO_ACK)
    {
        // 启动HTTPS Server
        if (this->st == STATE_DISCOVERING)
        {
            output = bufferevent_get_output(bev);
            this->st = STATE_SYNC_READY;
        }
        printf("[DEBUG] todo : add a tcp bufferevent to sync request!");

        return true;
    }
    else
        printf("%s : %d", "unsupport type, do not reply \n", header->type);
    return false;
}

static void errorcb(struct bufferevent *bev, short error, void *ctx)
{
    if (error & BEV_EVENT_EOF)
    {
        /* connection has been closed, do any clean up here */
        /* ... */
    }
    else if (error & BEV_EVENT_ERROR)
    {
        /* check errno to see what error occurred */
        /* ... */
    }
    else if (error & BEV_EVENT_TIMEOUT)
    {
        /* must be a timeout event handle, handle it */
        /* ... */
    }
    bufferevent_free(bev);
}

void readcb(struct bufferevent *bev, void *ctx)
{
    printf("recive msg \n");
    Discover *dis = (Discover *)ctx;
    dis->handle_hello_recv(bev);
}

void write_frequence_cb(evutil_socket_t fd, short events, void *arg)
{
    // todo (lutar) 引入定时器
    const sockaddr_in *target_sock_addr = (const sockaddr_in *)arg;
    printf("want to send udp to %s \n", inet_ntoa(target_sock_addr->sin_addr));
    send_to(fd, *target_sock_addr, "hello");
}

void Discover::start()
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock > 0);

    int optval = 1; // 这个值一定要设置，否则可能导致sendto()失败
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(int));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int));
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

    struct event *write_event;

    for (size_t i = 0; i < inf_infos_len; i++)
    {
        struct local_inf_info info = inf_infos[i];

        if (info.broad_addr.sin_addr.s_addr == 0)
            continue;

        struct sockaddr_in *target_sock_addr = &target_sock_addrs[i];
        configure_sock(target_sock_addr, info);

        write_event = event_new(base, sock, EV_WRITE, write_frequence_cb, target_sock_addr); // todo(lutar) 注意这里的&arg，这里应该会被释放，所以应该使用malloc，但现在没有
        assert(write_event);
        event_add(write_event, nullptr); // todo(lutar) 当接收到server的相应后，立即释放该event

        struct bufferevent *bev;
        evutil_make_socket_nonblocking(sock);
        bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, readcb, nullptr, errorcb, target_sock_addr); // todo(lutar) 注意这里的&arg，这里应该会被释放，所以应该使用malloc，但现在没有
        bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);

        break;
    }
    event_base_dispatch(base);
    // event_free(write_event);
}

int main(int argc, char const *argv[])
{
    struct event_base *base = event_base_new();
    assert(base);
    Discover discover(base);
    discover.start();
    return 0;
}