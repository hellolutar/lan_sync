#ifndef __SDN_SHARE_PROTOCOL_H
#define __SDN_SHARE_PROTOCOL_H

#include <cstdint>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>

#define DISCOVER_SERVER_UDP_PORT 58080
#define DISCOVER_SERVER_HTTP_PORT 58081

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

void *encapsulate(lan_discover_header_t header, void *data, int data_len)
{
    int total_len = sizeof(header) + data_len;
    lan_discover_header_t *ret_header = (lan_discover_header_t *)malloc(total_len);
    memcpy(ret_header, &header, sizeof(header));
    memcpy(ret_header + 1, data, data_len);
    return ret_header;
}

void debug_print_header(struct lan_discover_header *header, struct in_addr addr)
{
    char data[4096] = {0};
    memcpy(data, header + 1, header->data_len);
    data[header->data_len] = '\0';
    printf("[DEBUG] recive msg from :%s \n"
           "\t version: %d \n"
           "\t type: %d \n"
           "\t len: %d \n",
           inet_ntoa(addr),
           header->version,
           header->type,
           header->data_len);
}

enum lan_sync_version : uint8_t
{
    LAN_SYNC_VER_0_1 = 1,
};

enum lan_sync_type_enum : uint8_t
{
    LAN_SYNC_TYPE_GET_TABLE_INDEX = 1,
    LAN_SYNC_TYPE_GET_RESOURCE,
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

#endif