
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
}

Discover::~Discover()
{
    if (inf_infos)
        free(inf_infos);
    inf_infos = nullptr;
}

void handleLanSyncReplyTableIndex(struct evbuffer *in, struct evbuffer *out, lan_sync_header_t *try_header, int recvLen)
{
    int total_len = try_header->total_len;
    if (recvLen < total_len)
    {
        return;
    }
    printf("[DEBUG] [TCP] cli recive table index!\n");

    char *bufp = (char *)malloc(total_len);
    memset(bufp, 0, total_len);

    recvLen = evbuffer_remove(in, bufp, total_len);
    printf("[DEBUG] EVBUFFER REMAIN: %ld \n", evbuffer_get_length(in));
    assert(recvLen == total_len);

    lan_sync_header_t *header = (lan_sync_header_t *)bufp;

    // 提取记录数量
    int data_len = header->total_len - header->header_len;
    uint64_t res_num = data_len / sizeof(struct Resource);

    struct Resource *table = (struct Resource *)(++header);
    map<string, Resource> total_table;

    for (size_t i = 0; i < res_num; i++)
    {
        printf("name: %s\n", table[i].name);
        printf("size: %ld\n", table[i].size);
        printf("uri: %s\n", table[i].uri);
        printf("hash: %s\n", table[i].hash);
        total_table[table[i].name] = table[i];
        printf("\n");
    }

    vector<struct Resource *> local_table = discover->rm.getTable();
    for (size_t i = 0; i < local_table.size(); i++)
    {
        struct Resource *local_rs = local_table[i];
        struct Resource rs = total_table[local_rs->name];
        if (strlen(rs.name) == 0 || rs.size <= local_rs->size)
        {
            // I have the resource.
            total_table.erase(local_rs->name);
        }
    }

    for (auto iter = total_table.begin(); iter != total_table.end(); iter++)
    {
        discover->syncTable.push_back(WantSyncResource_new(out, iter->second.uri, PENDING));
    }
    free(bufp);
}

bool checkHash(lan_sync_header_t *header, string pathstr);

void handleLanSyncReplyResource(struct evbuffer *in, struct evbuffer *out, lan_sync_header_t *try_header, int recvLen)
{
    int total_len = try_header->total_len;
    if (recvLen < total_len)
    {
        return;
    }
    printf("[DEBUG] [TCP] cli recive resource!\n");

    char *bufp = (char *)malloc(total_len);
    memset(bufp, 0, total_len);

    recvLen = evbuffer_remove(in, bufp, total_len);
    assert(recvLen == total_len);

    lan_sync_header_t *header = (lan_sync_header_t *)bufp;

    string uri = lan_sync_header_query_xheader(header, XHEADER_URI);
    if (uri == "")
    {
        printf("[ERROR] [TCP] handleLanSyncReplyResource() query header is failed! \n");
        free(bufp);
        return;
    }

    // 写
    string pathstr = discover->rm.getRsHome() + uri;

    auto path = filesystem::path(pathstr);
    auto ppath = filesystem::absolute(path).parent_path();
    if (!filesystem::exists(ppath))
    {
        filesystem::create_directories(path.parent_path());
    }

    int fd = open(pathstr.data(), O_RDWR | O_CREAT, 0644);
    if (fd < 0)
    {
        printf("[ERROR] %s \n", strerror(errno));
        free(bufp);
        return;
    }

    int data_len = header->total_len - header->header_len;
    char *data = (char *)malloc(data_len);
    lan_sync_header_extract_data(header, data);
    // TODO(lutar) 这里需要对写出的数量进行处理
    int writed = write(fd, data, data_len); // todo
    printf("writed: %d \n", writed);

    close(fd);

    bool valid = checkHash(header, pathstr);
    if (valid)
    {
        auto table = discover->syncTable;
        for (auto iter = table.end() - 1; iter >= table.begin(); iter--)
        {
            WantSyncResource *rs = (*iter);
            if (rs->uri == uri)
            {
                rs->status = SUCCESS;
                break;
            }
        }
    }

    free(data);
    free(bufp);
}

bool checkHash(lan_sync_header_t *header, string pathstr)
{
    auto p = filesystem::path(pathstr);
    if (!filesystem::exists(p))
    {
        printf("[ERROR] [HASH CHECK] not exists: [%s]\n", pathstr.data());
        return false;
    }

    string hash = lan_sync_header_query_xheader(header, XHEADER_HASH);
    OpensslUtil opensslUtil;
    string theFileHash = opensslUtil.mdEncodeWithSHA3_512(pathstr.data());

    if (hash.compare(theFileHash) != 0)
    {
        printf("[ERROR] [HASH CHECK] hash is conflict! [%s]\n", pathstr.data());
        return false;
    }
    else
    {
        printf("[INFO ] [HASH CHECK] hash is valid! [%s]\n", pathstr.data());
        return true;
    }
}

void tcp_readcb(struct bufferevent *bev, void *ctx)
{
    printf("[DEBUG] [TCP] : receive msg!\n");
    struct evbuffer *in = bufferevent_get_input(bev);
    struct evbuffer *out = bufferevent_get_output(bev);

    int recvLen = evbuffer_get_length(in);
    if (recvLen == 0)
    {
        return;
    }

    char buf[lan_sync_header_len + 1] = {0};
    evbuffer_copyout(in, buf, lan_sync_header_len);
    lan_sync_header_t *try_header = (lan_sync_header_t *)buf;
    if (try_header->type == LAN_SYNC_TYPE_REPLY_TABLE_INDEX)
    {
        handleLanSyncReplyTableIndex(in, out, try_header, recvLen);
    }
    else if (try_header->type == LAN_SYNC_TYPE_REPLY_RESOURCE)
    {
        handleLanSyncReplyResource(in, out, try_header, recvLen);
    }
}

void tcp_writecb(struct bufferevent *bev, void *ctx)
{
    // printf("[DEBUG] can write msg to peer tcp server! \n");
}

void connect_tcp_server(struct cb_arg *arg)
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

    struct evbuffer *out = bufferevent_get_output(bev);

    lan_sync_header_t *header = lan_sync_header_new(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_TABLE_INDEX); // free in lan_sync_encapsulate --> evbuffer_cb_for_free

    lan_sync_encapsulate(out, header);
    printf("[INFO ] [TCP] [lan sync] req resource!\n");
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

    if (header->type == LAN_DISCOVER_TYPE_HELLO_ACK)
    {
        printf("[DEBUG] [UDP] recive [HELLO ACK], data_len:%d , port: %d\n", header->data_len, peer_tcp_port);

        // 启动HTTPS Server
        if (discover->st == STATE_DISCOVERING)
        {
            discover->st = STATE_SYNC_READY;

            // todo(lutar) 形成 RESOURCE_TABLE, 这里是不是也可以引入状态机

            struct cb_arg *arg = cb_arg_new(base);
            memcpy(arg->target_addr, &target_addr, sizeof(target_addr));
            arg->target_addr->sin_port = htons(peer_tcp_port);

            connect_tcp_server(arg);
            printf("[DEBUG] todo : add a tcp bufferevent to sync request!\n");
        }
    }
    else
        printf("[WARN] [UDP] recive [404]%s : %d", "unsupport type, do not reply \n", header->type);
}

static void timeout_cb(evutil_socket_t, short, void *arg)
{
    printf("[DEBUG] [UDP] send [HELLO]\n");

    udp_cli *cli = (udp_cli *)arg;

    string msg = "hello";
    lan_discover_header_t reply_header = {LAN_DISCOVER_VER_0_1, LAN_DISCOVER_TYPE_HELLO, sizeof(msg.data())};

    struct evbuffer *buf = evbuffer_new();
    evbuffer_add(buf, &reply_header, sizeof(lan_discover_header_t));
    evbuffer_add(buf, msg.data(), msg.size());

    cli->send(buf);
}

void Discover::config_send_udp_periodically(struct local_inf_info info)
{
    udp_cli *cli = new udp_cli(udp_sock, base, info.broad_addr.sin_addr);
    struct event *timeout_event = event_new(base, -1, EV_PERSIST, timeout_cb, cli);

    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = PERIOD_OF_SEND_UDP;
    event_add(timeout_event, &tv);
}

void handle_sync_status_pending(WantSyncResource *rs)
{
    printf("[INFO ] [SYNC] [%s] sync req rs! \n", rs->uri.data());
    rs->status = PENDING;
    lan_sync_header_t *header = lan_sync_header_new(LAN_SYNC_VER_0_1, LAN_SYNC_TYPE_GET_RESOURCE); // free in lan_sync_encapsulate --> evbuffer_cb_for_free
    header = lan_sync_header_add_xheader(header, XHEADER_URI, rs->uri);
    lan_sync_encapsulate(rs->out, header);
}

void handle_sync_status_syncing(WantSyncResource *rs)
{
    time_t now = time(0);
    int diff = difftime(now, rs->last_update_time);
    if (diff > 250)
    {
        printf("[INFO ] [SYNC] [%s] sync cost a lot of time! now reset status\n", rs->uri.data());
        rs->status = PENDING;
        rs->last_update_time = time(0);
    }
}

static void req_resource_periodically_timeout(evutil_socket_t, short, void *arg)
{
    vector<WantSyncResource *> table = discover->syncTable;
    int size = table.size();
    if (size == 0)
    {
        return;
    }

    for (auto iter = table.end() - 1; iter >= table.begin(); iter--)
    {
        WantSyncResource *rs = (*iter);
        switch (rs->status)
        {
        case PENDING:
            handle_sync_status_pending(rs);
            break;
        case SYNCING:
            handle_sync_status_syncing(rs);
            break;
        case SUCCESS:
            table.erase(iter);
            printf("[INFO ] [SYNC] [%s] sync done! \n", rs->uri.data());
            break;
        case FAIL:
            rs->status = PENDING;
            rs->last_update_time = time(0);
            printf("[INFO ] [SYNC] [%s] sync fail!  now reset status\n", rs->uri.data());
            break;
        default:
            printf("[WARN ] [SYNC] [%s] sync status is unsupport\n", rs->uri.data());
            break;
        }
    }
}

void config_req_resource_periodically()
{
    struct event *timeout_event = event_new(base, -1, EV_PERSIST, req_resource_periodically_timeout, nullptr);

    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = PERIOD_OF_REQ_RS;
    event_add(timeout_event, &tv);
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

    config_req_resource_periodically();

    for (size_t i = 0; i < inf_infos_len; i++)
    {
        struct local_inf_info info = inf_infos[i];

        if (info.broad_addr.sin_addr.s_addr == 0)
            continue;

        config_send_udp_periodically(info);

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