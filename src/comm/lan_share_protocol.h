#ifndef __LAN_SHARE_PROTOCOL_H
#define __LAN_SHARE_PROTOCOL_H

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <malloc.h>

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <event2/buffer.h>

#include "resource/resource.h"

using namespace std;

#define DISCOVER_SERVER_UDP_PORT 58080
#define DISCOVER_SERVER_TCP_PORT 58081

#define FLAG_KEY_VALUE_SPLIT 2 // 2 is: ':' and '\0'

#define XHEADER_URI "uri"
#define XHEADER_HASH "hash"
#define XHEADER_TCPPORT "tcpport"

class LocalPort
{
private:
    string name;
    struct sockaddr_in addr;
    struct sockaddr_in broad_addr;
    struct sockaddr_in subnet_mask;

public:
    LocalPort(/* args */);
    ~LocalPort();

    static vector<LocalPort> query();
    static bool existIp(vector<LocalPort> ports, struct in_addr addr);
    struct sockaddr_in getAddr();
    struct sockaddr_in getBroadAddr();
    struct sockaddr_in getSubnetMask();
};

enum state : uint8_t
{
    STATE_DISCOVERING = 1,
    STATE_SYNC_READY = 2,
};

enum lan_sync_version : uint8_t
{
    LAN_SYNC_VER_0_1 = 1,
};

enum lan_sync_type_enum : uint8_t
{
    LAN_SYNC_TYPE_HELLO = 1,
    LAN_SYNC_TYPE_HELLO_ACK,
    LAN_SYNC_TYPE_GET_TABLE_INDEX,
    LAN_SYNC_TYPE_REPLY_TABLE_INDEX,
    LAN_SYNC_TYPE_GET_RESOURCE,
    LAN_SYNC_TYPE_REPLY_RESOURCE,
    LAN_SYNC_TYPE_UPDATE_RESOURCE,
    LAN_SYNC_TYPE_CLOSE,
};

typedef struct lan_sync_header
{
    enum lan_sync_version version;
    enum lan_sync_type_enum type;
    uint16_t header_len;
    uint32_t total_len;
    
} lan_sync_header_t;

#define LEN_LAN_SYNC_HEADER_T sizeof(lan_sync_header_t)

struct Resource *lan_sync_parseTableToData(vector<struct Resource *> table);

struct cb_arg
{
    struct event_base *base;
    struct sockaddr_in *target_addr;
    struct evbuffer *buf;
};

struct cb_arg *cb_arg_new(struct event_base *base);

void cb_arg_free(struct cb_arg *arg);

void writecb(evutil_socket_t fd, short events, void *ctx);


class LanSyncPkt
{
private:
    map<string, string> xheader;
    void *data; //  ~LanSyncPkt()
    uint16_t header_len;
    uint32_t total_len;

public:
    enum lan_sync_version version;
    enum lan_sync_type_enum type;
    LanSyncPkt(enum lan_sync_version version, enum lan_sync_type_enum type)
        : version(version), type(type), header_len(LEN_LAN_SYNC_HEADER_T), total_len(LEN_LAN_SYNC_HEADER_T), data(nullptr){};
    LanSyncPkt(lan_sync_header_t *header);
    ~LanSyncPkt();

    void write(struct evbuffer *out);

    void addXheader(const string key, const string value);

    string queryXheader(string key);

    void *getData();

    void setData(void *data, uint32_t datalen);

    uint16_t getHeaderLen();
    uint32_t getTotalLen();
    enum lan_sync_version getVersion();
    enum lan_sync_type_enum getType();
    const map<string,string> getXheaders();
};



bool compareChar(char *l, char *r, uint32_t cnt);

#endif