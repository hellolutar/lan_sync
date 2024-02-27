#include <fcntl.h>

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <string>
#include <vector>

#include <unistd.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

// #include <spdlog/spdlog.h>

#include "comm/lan_share_protocol.h"
#include "resource/resource_manager.h"
#include "utils/logger.h"
#include "constants.h"
#include "comm/udp_cli.h"

using namespace std;

class SyncServer
{
private:
    void handleLanSyncGetTableIndex(struct evbuffer *in, struct evbuffer *out, lan_sync_header_t *try_header, uint32_t recvLen);
    void handleLanSyncGetResource(struct evbuffer *in, struct evbuffer *out, lan_sync_header_t *try_header, uint32_t recvLen);
    void replyResource(struct evbuffer *out, char *uri);

public:
    enum state st;
    struct event_base *base;
    int udp_sock;
    SyncServer(struct event_base *base);
    ~SyncServer();
    void start();
    void start_tcp_server(struct event_base *base);
    void handleTcpMsg(struct bufferevent *bev);
    void handleUdpMsg(struct sockaddr_in target_addr, char *data, int data_len);

    ResourceManager rm = ResourceManager("static/server");
};
