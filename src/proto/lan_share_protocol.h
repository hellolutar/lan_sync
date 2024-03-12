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

#include <event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#define DEFAULT_DISCOVER_SERVER_UDP_PORT 58080
#define DEFAULT_DISCOVER_SERVER_TCP_PORT 58081

#define FLAG_KEY_VALUE_SPLIT 2 // 2 is: ':' and '\0'

#define XHEADER_URI "uri"
#define XHEADER_HASH "hash"
#define XHEADER_TCPPORT "tcpport"
/**
 * format:
 *      content-range:${first byte pos}-${last byte pos}\0
 * example:
 *      range:0-500\0                   want to get [0, 500)
 *      range:0-\0                      want to get [0, total_size)
 */
#define XHEADER_RANGE "range"

/**
 * format:
 *      content-range:${first byte pos}-${last byte pos + 1 }/${entity legth}\0
 * example:
 *      content-range:0-500/500/last\0     reply[0,500), total size for the file is 500, this pkt is the last pkt
 *      content-range:0-500/22400/more\0     reply[0,500), total size for the file is 22400, the pkt is one of the many pkts sent by the server.
 */
#define XHEADER_CONTENT_RANGE "content-range"

#define FLAG_XHEADER_CONTENT_RANGE_LAST "last"
#define FLAG_XHEADER_CONTENT_RANGE_MORE "more"
#define FLAG_XHEADER_CONTENT_SEPERATE "/"
#define FLAG_XHEADER_CONTENT_SEPERATE_CHAR '/'
#define FLAG_XHEADER_CONTENT_BETWEEN "-"
#define FLAG_XHEADER_CONTENT_BETWEEN_CHAR '-'

#define SIZE_1KB 8196

class LocalPort
{
private:
    std::string name;
    struct sockaddr_in addr;
    struct sockaddr_in broad_addr;
    struct sockaddr_in subnet_mask;

public:
    LocalPort(/* args */);
    ~LocalPort();

    static std::vector<LocalPort> query();
    static bool existIp(std::vector<LocalPort> ports, struct in_addr addr);
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
    std::map<std::string, std::string> xheader;
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

    void write(struct bufferevent *bev);
    void write(struct evbuffer *out);

    void addXheader(const std::string key, const std::string value);

    std::string queryXheader(std::string key);

    void *getData();

    void setData(void *data, uint32_t datalen);

    uint16_t getHeaderLen();
    uint32_t getTotalLen();
    uint32_t getDataLen();
    enum lan_sync_version getVersion();
    enum lan_sync_type_enum getType();
    const std::map<std::string, std::string> getXheaders();
};

bool compareChar(char *l, char *r, uint32_t cnt);

class ContentRange
{
private:
    uint64_t start_pos;
    uint64_t size;
    uint64_t total_size;
    bool is_last;

public:
    // str like : 0-500/500/last
    ContentRange(std::string str);
    ContentRange(uint64_t start_pos, uint64_t size, uint64_t total_size, bool is_last)
        : start_pos(start_pos), size(size), total_size(total_size), is_last(is_last){};
    ~ContentRange();

    uint64_t getStartPos();
    uint64_t getSize();
    uint64_t getTotalSize();
    bool isLast();
    std::string to_string();
};

class Range
{
private:
    uint64_t start_pos;
    uint64_t size;

public:
    Range(std::string str);
    Range(uint64_t start_pos, uint64_t size) : start_pos(start_pos), size(size){};
    ~Range();
    uint64_t getStartPos();
    uint64_t getSize();
    std::string to_string();

    static std::string defaultStr;
};

#endif