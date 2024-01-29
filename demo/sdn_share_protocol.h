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

enum state
{
    DISCOVERING = 1,
    SYNC_READY,
};

enum sdn_proto_version
{
    v0_1 = 1,
};

enum sdn_proto_type_enum
{
    HELLO = 1,
    HELLO_ACK = 2,
    SYNC_INDEX = 3,
};

typedef struct sdn_share_proto_header
{
    uint8_t version;
    uint8_t type;
    uint16_t data_len; // data lenth
} sdn_share_proto_header_t;

void *encapsulate(sdn_share_proto_header_t header, void *data, int data_len)
{
    int total_len = sizeof(header) + data_len;
    sdn_share_proto_header_t *ret_header = (sdn_share_proto_header_t *)malloc(total_len);
    memcpy(ret_header, &header, sizeof(header));
    memcpy(ret_header + 1, data, data_len);
    return ret_header;
}

void debug_print_header(struct sdn_share_proto_header *header, struct in_addr addr)
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

#endif