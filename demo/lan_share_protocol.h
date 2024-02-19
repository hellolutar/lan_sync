#ifndef __LAN_SHARE_PROTOCOL_H
#define __LAN_SHARE_PROTOCOL_H

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <malloc.h>

#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>

#include <event2/buffer.h>

#include "resource.h"

using namespace std;

#define DISCOVER_SERVER_UDP_PORT 58080
#define DISCOVER_SERVER_TCP_PORT 58081

struct local_inf_info
{
    char name[IFNAMSIZ];
    struct sockaddr_in addr;
    struct sockaddr_in broad_addr;
    struct sockaddr_in subnet_mask;
};

enum state : uint8_t
{
    STATE_DISCOVERING = 1,
    STATE_SYNC_READY = 2,
};

enum lan_discover_version : uint8_t
{
    LAN_DISCOVER_VER_0_1 = 1,
};

enum lan_discover_type_enum : uint8_t
{
    LAN_DISCOVER_TYPE_HELLO = 1,
    LAN_DISCOVER_TYPE_HELLO_ACK,
};

typedef struct lan_discover_header
{
    enum lan_discover_version version;
    enum lan_discover_type_enum type;
    uint16_t data_len;
} lan_discover_header_t;

void *lan_discover_encapsulate(lan_discover_header_t header, void *data, int data_len);

void debug_print_header(struct lan_discover_header *header, struct in_addr addr);

enum lan_sync_version : uint8_t
{
    LAN_SYNC_VER_0_1 = 1,
};

enum lan_sync_type_enum : uint8_t
{
    LAN_SYNC_TYPE_GET_TABLE_INDEX = 1,
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
    uint64_t data_len;
} lan_sync_header_t;

void lan_sync_encapsulate(struct evbuffer *out, lan_sync_header_t header, void *data, int data_len);

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

#endif