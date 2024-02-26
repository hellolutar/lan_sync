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
    uint16_t total_len;
} lan_sync_header_t;

#define lan_sync_header_len sizeof(lan_sync_header_t)

void lan_sync_encapsulate(struct evbuffer *out, lan_sync_header_t *header);

struct Resource *lan_sync_parseTableToData(vector<struct Resource *> table);

struct cb_arg
{
    struct event_base *base;
    struct sockaddr_in *target_addr;
    struct evbuffer *buf;
};

struct cb_arg *cb_arg_new(struct event_base *base);

void cb_arg_free(struct cb_arg *arg);

lan_sync_header_t *lan_sync_header_new(enum lan_sync_version version, enum lan_sync_type_enum type);

void writecb(evutil_socket_t fd, short events, void *ctx);

lan_sync_header_t *lan_sync_header_set_data(lan_sync_header_t *header, void *data, int datalen);

lan_sync_header_t *lan_sync_header_add_xheader(lan_sync_header_t *header, const string key, const string value);

void lan_sync_header_extract_xheader(const lan_sync_header_t *header, char *to);

string lan_sync_header_query_xheader(const lan_sync_header_t *header, string key);

void lan_sync_header_extract_data(const lan_sync_header_t *header, char *to);

#endif