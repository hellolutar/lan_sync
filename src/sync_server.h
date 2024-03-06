#ifndef __SYNC_SERVER_H_
#define __SYNC_SERVER_H_

#include <fcntl.h>

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <cstdlib>

#include <string>
#include <vector>
#include <algorithm>

#include <unistd.h>
#include <netinet/tcp.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "proto/lan_share_protocol.h"
#include "resource/resource_manager.h"
#include "utils/logger.h"
#include "utils/io_utils.h"
#include "constants.h"
#include "proto/udp_cli.h"
#include "components/sync_io_read_monitor.h"

#include <cstring>

#include "net/network_layer_with_event.h"

class SyncLogic;

class SyncUdpServer : public NetworkEndpoint
{
private:
    SyncLogic *logic;

public:
    SyncUdpServer(struct sockaddr_in *addr) : NetworkEndpoint(addr){};
    ~SyncUdpServer();

    void recv(void *data, uint64_t data_len, NetworkContext *ctx);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
    void setLogic(SyncLogic *logic);
};

class SyncTcpServer : public NetworkEndpoint
{
private:
    SyncLogic *logic;

public:
    SyncTcpServer(struct sockaddr_in *addr) : NetworkEndpoint(addr){};
    ~SyncTcpServer();

    void recv(void *data, uint64_t data_len, NetworkContext *ctx);
    bool isExtraAllDataNow(void *data, uint64_t data_len);
    void setLogic(SyncLogic *logic);
};

class SyncLogic
{
private:
    NetworkEndpoint *udpserver;
    NetworkEndpoint *tcpserver;

public:
    enum state st;
    ResourceManager rm = ResourceManager("static/server");

    SyncLogic(NetworkEndpoint *udpserver, NetworkEndpoint *tcpserver);
    ~SyncLogic();
    void handleTcpMsg(struct bufferevent *bev);
    void recvUdp(void *data, uint64_t data_len, NetworkContext *ctx);
    void recvTcp(void *data, uint64_t data_len, NetworkContext *ctx);

    void replyTableIndex(NetworkContext *ctx);
    void replyResource(lan_sync_header_t *header, NetworkContext *ctx);
};

#endif
