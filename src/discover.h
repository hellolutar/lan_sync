#ifndef __DISCOVER_H
#define __DISCOVER_H

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cerrno>
#include <stdlib.h>

#include <string>
#include <vector>
#include <map>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "comm/lan_share_protocol.h"
#include "resource/resource_manager.h"
#include "comm/udp_cli.h"
#include "utils/logger.h"
#include "constants.h"

#define PERIOD_OF_SEND_UDP 5
#define PERIOD_OF_REQ_TABLE_INDEX 10

#define PERIOD_OF_REQ_RS 5

using namespace std;

typedef uint32_t in_addr_t;

class Discover
{
private:
    void config_udp_sock();
    void config_send_udp_periodically();
    void config_req_resource_periodically();
    void config_req_table_index_periodically();
    void handleHelloAck(struct sockaddr_in target_addr, struct lan_discover_header *header);
    void connPeerWithTcp(struct sockaddr_in target_addr, uint16_t peer_tcp_port);
    void handle_sync_status_syncing(WantSyncResource *rs);
    void handle_sync_status_pending(WantSyncResource *rs);
    void handleLanSyncReplyResource(struct bufferevent *bev, lan_sync_header_t *try_header, int recvLen);
    void handleLanSyncReplyTableIndex(struct bufferevent *bev, lan_sync_header_t *try_header, int recvLen);
    void appendSyncTable(struct Resource *table, struct bufferevent *bev, uint64_t res_num);
    bool checkHash(lan_sync_header_t *header, string pathstr);
    map<uint32_t, struct bufferevent *> tcpTable;

public:
    enum state st;
    struct event_base *base;
    int udp_sock;
    ResourceManager rm = ResourceManager("static/cli");
    vector<WantSyncResource *> syncTable;

    void addSyncResource(struct WantSyncResource *);
    void delSyncResource(string uri);
    void updateSyncResourceStatus(string uri, enum WantSyncResourceStatusEnum);

    Discover(struct event_base *base);
    ~Discover();
    void do_connect(evutil_socket_t sock, short event, void *arg);
    void init();
    void start();
    void handleUdpMsg(struct sockaddr_in target_addr, char *data, int data_len);
    void handleTcpMsg(struct bufferevent *bev);
    void addTcpSession(in_addr_t, struct bufferevent *bev);
    void delTcpSessionByBufevent(struct bufferevent *bev);
    bool existTcpSessionByBufevent(struct bufferevent *bev);
    bool existTcpSessionByAddr(in_addr_t target_addr);
    map<in_addr_t, struct bufferevent *> getTcpTable();
    void syncResource();
    void reqTableIndex();
};

#endif